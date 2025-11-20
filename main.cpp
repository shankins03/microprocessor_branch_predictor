#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim_bp.h"
#include <vector>
#include <cmath>

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim bimodal 6 gcc_trace.txt
    argc = 4
    argv[0] = "sim"
    argv[1] = "bimodal"
    argv[2] = "6"
    ... and so on
*/

class Predictor {
public:
    int num_for_index;
    int num_for_BHR;
    char taken;
    int num_predict;
    int num_mispredict;
    int branch_history_reg;
//    int flip_branch_history_reg;
//    bool first_shift;
    std::vector<int> prediction_table;


    Predictor(int m_bits, int n_bits)
    {
        num_for_index = m_bits;
        num_for_BHR = n_bits;
        num_predict = 0;
        num_mispredict = 0;
        branch_history_reg = 0;
//        flip_branch_history_reg = (1 << num_for_BHR) - 1;
        prediction_table.resize(pow(2,num_for_index), 2);
//        first_shift = true;
    }

    // Returns index for hex branch bimodal
    int index_counter_bimodal(int hex_branch)
    {
        int index_val = (hex_branch >> 2) & ((1 << num_for_index) - 1);
        return index_val;
    }

    int index_counter_gshare(int hex_branch)
    {
        int shifted = branch_history_reg << (num_for_index-num_for_BHR);
        int index_val = (hex_branch >> 2) & ((1 << num_for_index) - 1);
        index_val = index_val ^ shifted;
        return index_val;
    }

    // returns 'n' if not taken, 't' if taken
    void branch_prediction(int index)
    {
        if(prediction_table[index] < 2) taken = 'n';
        else taken = 't';
    }

    void update_BHR(char result)
    {
        if(result == 't') {
//            flip_branch_history_reg >> 1;
            branch_history_reg = (branch_history_reg >> 1) + (1 << num_for_BHR-1);
        }
        else {
//            flip_branch_history_reg
            branch_history_reg = (branch_history_reg >> 1);
        }
    }

    void update_predictor(char prediction, char result, int index)
    {
        num_predict++;
        if(result == 't' && prediction_table[index] < 3)
        {
            if(prediction == 'n') num_mispredict++;
            prediction_table[index]++;
        }
        else if (result == 'n' && prediction_table[index] > 0)
        {
            if(prediction == 't') num_mispredict++;
            prediction_table[index]--;
        }

    }

};

int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    bp_params params;       // look at sim_bp.h header file for the the definition of struct bp_params
    char outcome;           // Variable holds branch outcome
    unsigned long int addr; // Variable holds the address read from input file

    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }

    params.bp_name  = argv[1];

    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
    {
        if(argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M2       = strtoul(argv[2], NULL, 10);
        trace_file      = argv[3];
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
    }
    else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare
    {
        if(argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M1       = strtoul(argv[2], NULL, 10);
        params.N        = strtoul(argv[3], NULL, 10);
        trace_file      = argv[4];
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);

    }
    else if(strcmp(params.bp_name, "hybrid") == 0)          // Hybrid
    {
        if(argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.K        = strtoul(argv[2], NULL, 10);
        params.M1       = strtoul(argv[3], NULL, 10);
        params.N        = strtoul(argv[4], NULL, 10);
        params.M2       = strtoul(argv[5], NULL, 10);
        trace_file      = argv[6];
        printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);

    }
    else
    {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }

    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }

    Predictor* branch_predictor = nullptr;
    if (strcmp(params.bp_name, "bimodal") == 0) branch_predictor = new Predictor(params.M2, 0);
    else if(strcmp(params.bp_name, "gshare") == 0) branch_predictor = new Predictor(params.M1, params.N);

//    printf("\n Branch predictor created!\n");

    char str[2];
    while(fscanf(FP, "%lx %s", &addr, str) != EOF)
    {

        outcome = str[0];
//        if (outcome == 't')
//            printf("%lx %s\n", addr, "t");           // Print and test if file is read correctly
//        else if (outcome == 'n')
//            printf("%lx %s\n", addr, "n");          // Print and test if file is read correctly

        /*************************************
            Add branch predictor code here
        **************************************/
        int index_hold = 0;
        char char_prediction;
        if (strcmp(params.bp_name, "bimodal") == 0) index_hold = branch_predictor->index_counter_bimodal(addr);
        else if (strcmp(params.bp_name, "gshare") == 0) index_hold = branch_predictor->index_counter_gshare(addr);
        branch_predictor->branch_prediction(index_hold);
        char_prediction = branch_predictor->taken;
        branch_predictor->update_predictor(char_prediction, outcome, index_hold);
        branch_predictor->update_BHR(outcome);
    }

//    printf("COMMAND");
//    if (strcmp(params.bp_name, "bimodal") == 0) printf("\n./sim %s %i %s", params.bp_name, params.M2, trace_file);
//    else if (strcmp(params.bp_name, "gshare") == 0) printf("\n./sim %s %i %i %s", params.bp_name, params.M1, params.N, trace_file);
    printf("\nOUTPUT");
    printf("\nnumber of predictions:    %i", branch_predictor->num_predict);
    printf("\nnumber of mispredictions: %i", branch_predictor->num_mispredict);
    printf("\nmisprediction rate:       %.2f%%", 100*(float) branch_predictor->num_mispredict / (float) branch_predictor->num_predict);
    if (strcmp(params.bp_name, "bimodal") == 0) printf("\nFINAL BIMODAL CONTENTS");
    else if (strcmp(params.bp_name, "gshare") == 0) printf("\nFINAL GSHARE CONTENTS");

    for(int i=0; i < branch_predictor->prediction_table.size(); i++) {
        printf("\n%i\t%i", i, branch_predictor->prediction_table[i]);
    }

    return 0;
}
