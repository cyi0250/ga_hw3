#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <time.h>
#include <sys/time.h>
#include <fstream>

#define STOC(X) X - 1
#define CTOS(X) X + 1

using namespace std;

struct EDGE{
    struct VERT * a;
    struct VERT * b;
    int w;
};

struct VERT{
    int vert_id;
    int vert_grp;
};

struct EL{
    int score;
    int* bitstring;
};

int ctoi(char c){
    int n = c - '0';
    return n;
}

float rand0to1(){
    float r = ((float) rand() / (RAND_MAX));
    return r;
}

int rand0or1(){
    int r = rand()%2;
    return r;
}

int compare_ls(int *a,int *b, int sz){
    int same = 1;
    for (int i = 0; i<sz; i++){
        if(a[i] != b[i]){
            same = 0;
            break;
        }
    }
    return same;
}

void mutation(struct EL *el, float r_mutation, int n_bits){
    for (int i = 0 ;i < n_bits; i++){
        if (rand0to1() < r_mutation)
            el->bitstring[i] = 1 - el->bitstring[i];
    }
}

void initialization(struct EL *elem, int n_bits){
    int *ptr = new int[n_bits];
    for (int i = 0; i < n_bits; i++){
        ptr[i] = rand0or1();
    }
    elem->bitstring = ptr;
    elem->score = 0;
}


int evaluate(int*bitstring, int n_bits, int n_edge, struct VERT l_v[], struct EDGE l_e[]){
    int score = 0;
    for(int i=0; i<n_bits; i++){
        l_v[i].vert_grp = bitstring[i];
    }
    for(int i=0; i<n_edge; i++){
        int a_grp = l_e[i].a->vert_grp;
        int b_grp = l_e[i].b->vert_grp;
        if(a_grp == b_grp){
          continue;
        }
        else{
          score = score + l_e[i].w;
        }
    }
    return score;
}

void evaluate_el(struct EL * el, int n_bits, int n_edge, struct VERT l_v[], struct EDGE l_e[]){
    el->score = evaluate(el->bitstring, n_bits, n_edge, l_v, l_e);
}

void evaluate_all(struct EL * pop, int n_bits, int n_edge, struct VERT l_v[], struct EDGE l_e[], int n_pop){
    for(int i = 0; i <n_pop; i++){
        evaluate_el(&pop[i], n_bits, n_edge, l_v, l_e);
    }
}

int select(struct EL * pop, int n_pop, int k){
    int sel_idx = rand()%n_pop;
    int candidate[k-1];
    for (int i = 0; i < k-1; i++){
        int idx = rand()%n_pop;
        if(pop[idx].score > pop[sel_idx].score){
            sel_idx = idx;
        }
    }
    return sel_idx;
}

void crossover(struct EL * p1, struct EL * p2, struct EL * result, float r_cross, int n_bits, int kpoint){
    int *c1_ptr = new int[n_bits];
    int *c2_ptr = new int[n_bits];
    
    for(int i = 0; i < n_bits; i++){
        c1_ptr[i] = p1->bitstring[i];
        c2_ptr[i] = p2->bitstring[i];
    }
    int j = 0;

    if (rand0to1() < r_cross){
        while(j < kpoint){
            int pt = (rand()%(n_bits-1))+1;
            for(int i = 0; i < pt; i++){
                c1_ptr[i] = p2->bitstring[i];
                c2_ptr[i] = p1->bitstring[i];
            }
            j++;
        }
    }
    result[0].bitstring = c1_ptr;
    result[1].bitstring = c2_ptr;
}


int get_highest_idx(struct EL * pop, int n_pop){
    float highest_score = pop[0].score;
    int idx = 0;
    for(int i = 0; i <n_pop; i++){
        float tmp_score = pop[i].score;
        if (tmp_score > highest_score){
            highest_score = tmp_score;
            idx = i;
        }
    }
    return idx;
}

float get_average_score(struct EL * pop, int n_pop){
    float total_score = 0;
    for(int i = 0; i <n_pop; i++){
        total_score += pop[i].score;
    }
    return float(total_score/n_pop);
}

struct EL genetic_algorithm(struct VERT vert_ls[], struct EDGE edge_ls[], int n_vert, int n_edge,
                            int n_iter, int n_population, float r_cross, float r_mutation, int k, int kpoint){
    time_t start, end;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    start = tv.tv_sec;

    struct EL * pop;
    struct EL * init = new struct EL[n_population];
    pop = init;

    //Initialize population
    for (int i = 0; i<n_population; i++){
        initialization(&pop[i],n_vert);
    }

    for(int gen = 0; gen<n_iter; gen++){
        gettimeofday(&tv, NULL);
        end = tv.tv_sec;
        if(end - start >= 59){
            //cout<<"TIMEOVER"<<endl;
            break;
        }
        evaluate_all(pop, n_vert, n_edge, vert_ls, edge_ls, n_population);
        // if(gen%1000==0){
        //   int h_idx;
        //   h_idx = get_highest_idx(pop,n_population);
        //   cout<<"gen: "<<gen<<" score: "<<pop[h_idx].score<<endl;
        //   for(int i =0; i<n_vert; i++){
        //       cout<<pop[h_idx].bitstring[i];
        //   }
        //   cout<<endl;
        // }
        //select parents
        int parent_idx[n_population];
        struct EL * children = new struct EL[n_population];
        for(int i = 0; i < n_population; i++){
            parent_idx[i] = select(pop,n_population,k);
        }
        //do crossover and mutation
        for(int i = 0; i < n_population; i=i+2){
            struct EL *p1 = &pop[parent_idx[i]];
            struct EL *p2 = &pop[parent_idx[i+1]];
            crossover(p1, p2, &children[i], r_cross, n_vert, kpoint);
            for(int j = 0; j < 2; j++){
                mutation(&children[i+j],r_mutation,n_vert);
            }
        }

        //delete heap allocated mem
        for(int i =0; i<n_population; i++){
            delete[](pop[i].bitstring);
            pop[i].bitstring = NULL;
        }
        delete[] pop;

        //replacement
        pop = children;
    }
    evaluate_all(pop, n_vert, n_edge, vert_ls, edge_ls, n_population);
    int highest_idx = get_highest_idx(pop, n_population);
    struct EL * highest_elem = &pop[highest_idx];
    //cout<<highest_elem->score<<" ";
    for(int i =0; i<n_vert; i++){
        if(highest_elem->bitstring[i] == 1){
            cout << i+1 << " ";
        }
    }
    cout<<endl;

    //delete heap allocated variables
    for(int i =0; i<n_population; i++){
        delete[](pop[i].bitstring);
        pop[i].bitstring = NULL;
    }
    delete[] pop;
}


int main(int argc, char**argv) {
    srand(time(NULL));

    string n_vert_s, n_edge_s;

    cin >> n_vert_s;
    cin >> n_edge_s;
    int n_vert, n_edge;

    n_vert = stoi(n_vert_s);
    n_edge = stoi(n_edge_s);

    int n_bits = n_vert;
    int n_iter = 9999999;
    int n_population = 2000;
    float r_cross = 0.9;
    float r_mutation = 1 / float(n_bits);
    int k = 4;
    int kpoint = n_vert/50;

    if (r_mutation > 0.03)
        r_mutation = 0.03;
    if (kpoint < 1)
        kpoint = 1;

    //struct EDGE and VERTs
    string tmp_a, tmp_b, tmp_w;
    struct EDGE edge_ls[n_edge];
    struct VERT vert_ls[n_vert];
    
    for(int i =0; i<n_vert; i++){
        vert_ls[i].vert_id=CTOS(i);
        vert_ls[i].vert_grp=-1;
    }
    for(int i =0; i<n_edge; i++){
        cin >> tmp_a;
        cin >> tmp_b;
        cin >> tmp_w;
        edge_ls[i].a = &vert_ls[STOC(stoi(tmp_a))];
        edge_ls[i].b = &vert_ls[STOC(stoi(tmp_b))];
        edge_ls[i].w = stoi(tmp_w);
    }

    //for(int i = 0; i < 30; i++) 
    genetic_algorithm(vert_ls, edge_ls, n_vert, n_edge, n_iter, n_population, r_cross, r_mutation, k, kpoint);
 
    /*ifstream fin("./proj2_sample_sol/sol_unweighted_50.txt");

    int *idx = (int*)malloc(sizeof(int)*512);
    int count = 0;

    while(!fin.eof()){    
        int vertex;
        fin >> vertex;
        idx[count] = vertex;
        count++;
    }
    count = count - 1;
    
    cout << count << endl;

    idx = (int*)realloc(idx, sizeof(int)*count);
    int sol[n_vert] = {0, };
    for(int i = 0; i < count; i++){
        int sol_idx = STOC(idx[i]);
        sol[sol_idx] = 1;
    }
    int sol_score = evaluate(sol, n_vert, n_edge, vert_ls, edge_ls);

    int idx_unweighted_50[] = {1 ,4, 5, 6, 8, 10,11, 12, 14, 15, 17, 21, 22, 23, 27, 28, 29, 30, 31, 34, 37, 39, 46, 48, 50};
    int sol_unweighted_50[50] = {0, };
    for(int i=0; i<sizeof(idx_unweighted_50)/sizeof(*idx_unweighted_50); i++){
        int sol_idx = STOC(idx_unweighted_50[i]);
        sol_unweighted_50[sol_idx] = 1;
    }
    int sol_score = evaluate(sol_unweighted_50,n_vert,n_edge,vert_ls,edge_ls);*/

    // cout<<"OPTIMIZED SCORE: "<<sol_score<<endl;
}
