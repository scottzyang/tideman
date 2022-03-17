#include <cs50.h>
#include <stdio.h>
#include <strings.h>

// Max number of candidates
#define MAX 9

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX]; 

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
}
pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Function prototypes
bool vote(int rank, string name, int ranks[]);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
bool cycle_averted(pair current_pair, int winner);
void print_winner(void);

int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = 0; j < candidate_count; j++)
        {
            locked[i][j] = false;
        }
    }

    pair_count = 0;
    int voter_count = get_int("Number of voters: ");

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];

        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);

            if (!vote(j, name, ranks))
            {
                printf("Invalid vote.\n");
                return 3;
            }
        }

        record_preferences(ranks);

        printf("\n");
    }

    add_pairs();
    sort_pairs();
    lock_pairs();
    print_winner();
    return 0;
}

// Update ranks given a new vote
bool vote(int rank, string name, int ranks[])
{
    for (int i = 0; i < candidate_count; i++)
    {
        if (strcasecmp(candidates[i], name) == 0) // Search for a matching name
        {
            ranks[rank] = i; // update ranks array with candidate index
            return true;
        }
    }
    return false;
}

// Update preferences given one voter's ranks
void record_preferences(int ranks[])
{
    for (int i = 0; i < candidate_count; i++)
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            preferences[ranks[i]][ranks[j]]++; // populates preferences array spot by spot based on candidate number
        }
    }
    return;
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void)
{
    for (int i = 0; i < candidate_count; i++) 
    {
        for (int j = i + 1; j < candidate_count; j++)
        {
            // swaps i and j to compare opposite pairs, compares in order, populates pairs array 
            if (preferences[i][j] > preferences[j][i]) // determines if i wins over j 
            {
                pairs[pair_count].winner = i;
                pairs[pair_count].loser = j;
                pair_count++;
            }
            else if (preferences[i][j] < preferences[j][i]) // determines if j wins over i
            {
                pairs[pair_count].winner = j;
                pairs[pair_count].loser = i;
                pair_count++;
            }
        }
    }
    return;
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    bool sorted = false;
    
    // loop through the pairs array multiple times until the list is sorted
    while (!sorted)
    {
        sorted = true;
        for (int i = 0; i < pair_count - 2; i++)
        {
            int current_winner = pairs[i].winner;
            int current_loser = pairs[i].loser;
            int next_winner = pairs[i + 1].winner;
            int next_loser = pairs[i + 1].loser;
            
            // deteremines margin of victory by subtracting opposite pairs
            int current_margin = preferences[current_winner][current_loser] - preferences[current_loser][current_winner];
            int next_margin = preferences[next_winner][next_loser] - preferences[next_loser][next_winner];
            
            // sorts pairs array by descending margin of victory
            if (current_margin < next_margin)
            {
                pair tmp = pairs[i];
                pairs[i] = pairs[i + 1];
                pairs[i + 1] = tmp;
                sorted = false;  // returns false to ensure the while loop still triggers to sort list
            }
        }
    }
    return;
}

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{
    for (int i = 0; i < pair_count; i++)
    {
        bool safe_lock = cycle_averted(pairs[i], pairs[i].winner); // takes in pair and the pair winner to evaluate if it is safe to lock
        
        if (safe_lock)
        {
            locked[pairs[i].winner][pairs[i].loser] = true;
        }
    }
    return;
}

// checks if it is safe to lock current pair being evaluated
bool cycle_averted(pair current_pair, int winner)
{
    bool no_cycle = true;
    
    for (int i = 0; i < pair_count; i++)
    {   
        pair next_pair = pairs[i]; // next pair is to be evaluated against current pair
        
        if (current_pair.loser == next_pair.winner) // checks if current loser exists as winner in pairs array
        {
            if (!locked[next_pair.winner][next_pair.loser]) // if current loser exists as winner, checks if the next winner pair is not locked
            {
                no_cycle = true; // if next winner pair is not locked, then mark it as true
            }
            else if (winner == next_pair.loser) // checks if original winner exists as a loser in pairs array
            {
                return false; // if original winner exists as loser, then a cycle is created
            }
            else
            {
                // if next pair loser does not equal current winner, then run the function again through all sorted pairs
                if (!cycle_averted(next_pair, winner)) 
                {
                    return false;
                }
            }
        }
    }
    return no_cycle;
}

// Print the winner of the election
void print_winner(void)
{
    for (int i = 0; i < candidate_count; i++)
    {
        int winning_candidate = i; // current candidate to evaluate
        bool no_edge = true; // assumes that current candidate is winner
        
        for (int j = 0; j < candidate_count; j++)
        {
            if (locked[j][i]) // evaluates if current candidate loses to other candidates
            {
                no_edge = false;
                break; // if current candidate loses, break loop and go back to outer loop to next candidate
            }
        }
        
        if (no_edge) // if candidate does not lose, then it is the winner
        {
            printf("%s\n", candidates[winning_candidate]);
            return;
        }
    }        
    return;
}

