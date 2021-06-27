#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include <sys/types.h>
//Estrutura para retorno das funções
typedef struct ret Ret;
struct ret{
    int b; //Bucket
    int s; //Slot
    int d; //Profundidade local
};
// ESTRUTURAS DO ÍNDICE
typedef struct bucket Bucket;
typedef struct slot Slot;
typedef struct directory Directory;

//Slot
struct slot{
    int64_t chave;
    int64_t rid;
};

//Bucket
struct bucket {
    int local_depth;
    Slot *slot1;
    Slot *slot2;
};

//Diretório
struct directory{
    int global_depth;
    Bucket **dir;
};

//FUNÇÕES

//Criar Slot
Slot *createSlot(void){
    Slot *s = (Slot*) malloc(sizeof(Slot));
    s->chave = INT64_MAX;
    s->rid = INT64_MAX;
    return s;
}

//Criar Bucket
Bucket *createBucket(int init_local_depth){
    Bucket *b = (Bucket*) malloc(sizeof(Bucket));
    b->local_depth = init_local_depth;
    b->slot1 = createSlot();
    b->slot2 = createSlot();
    return b;
}

//Criar Diretório
Directory *createDir(void){
    Directory *d = (Directory*) malloc(sizeof(Directory));
    d->global_depth = 2;
    d->dir = malloc(4*sizeof(Bucket*));
    Bucket **buckets = d->dir;
    buckets[0] = createBucket(2);
    buckets[1] = createBucket(2);
    buckets[2] = createBucket(2);
    buckets[3] = createBucket(2);
    return d;
}

//Dobrar Diretório
Directory *doubleDir(Directory *d){
    d->global_depth = d->global_depth + 1;
    int new_size = pow(2,d->global_depth);
    d->dir = (Bucket**) realloc(d->dir, new_size * sizeof(Bucket*));
    int first_new = pow(2,d->global_depth - 1);
    int last_new = pow(2,d->global_depth) - 1;
    int i;
    for (i = first_new; i <= last_new; ++i){
        d->dir[i] = d->dir[i-first_new];
    }
    return d;
}

//Inclusão de entrada de dados
Ret includeCompleto(Directory* d, int64_t k, int64_t r){
    Ret returned;
    Bucket **direc= d->dir;
    int gd = d->global_depth;
    int t = pow(2,gd);
    int h = k % t;
    Bucket *b1 = direc[h];
    Slot *s11 = b1->slot1;
    Slot *s12 = b1->slot2;
    if (s11->chave == INT64_MAX){
        s11->chave = k;
        s11->rid = r;
        returned.b = h;
        returned.s = 1;
        returned.d = b1->local_depth;
        return returned;
    }
    if (s12->chave == INT64_MAX){
        s12->chave = k;
        s12->rid = r;
        returned.b = h;
        returned.s = 2;
        returned.d = b1->local_depth;
        return returned;
    }
    if (b1->local_depth < gd){
        b1->local_depth = b1->local_depth + 1;
        Bucket *b2 = createBucket(b1->local_depth);
        int n;
        if (h < t/2){
            n = h + (t/2);
        } else{
            n = h - (t/2);
        }
        direc[n] = b2;
        Slot *s21 = b2->slot1;
        Slot *s22 = b2->slot2;
        int r1 = (s11->chave) % t;
        int r2 = (s12->chave) % t;
        if (r1 == n){
            s21->chave = s11->chave;
            s21->rid = s11->rid;
            s11->chave = INT64_MAX;
            s11->rid = INT64_MAX;
        }
        if (r2 == n){
            s22->chave = s12->chave;
            s22->rid = s12->chave;
            s12->chave = INT64_MAX;
            s12->rid = INT64_MAX;
        }
        returned = includeCompleto(d, k, r);
        return returned;
    } else if (b1->local_depth == gd){
        d = doubleDir(d);
        t = 2*t;
        b1->local_depth = b1->local_depth + 1;
        Bucket *b2 = createBucket(b1->local_depth);
        int n;
        if (h < t/2){
            n = h + (t/2);
        } else{
            n = h - (t/2);
        }
        direc = d->dir;
        direc[n] = b2;
        Slot *s21 = b2->slot1;
        Slot *s22 = b2->slot2;
        int r1 = (s11->chave) % t;
        int r2 = (s12->chave) % t;
        if (r1 == n){
            s21->chave = s11->chave;
            s21->rid = s11->rid;
            s11->chave = INT64_MAX;
            s11->rid = INT64_MAX;
        }
        if (r2 == n){
            s22->chave = s12->chave;
            s22->rid = s12->chave;
            s12->chave = INT64_MAX;
            s12->rid = INT64_MAX;
        }
        returned = includeCompleto(d, k, r);
        return returned;
    }
}

//Busca de entrada
Ret buscar(Directory *d, int64_t k){
    Ret returned;
    returned.b = -1;
    returned.s = -1;
    int gd = d->global_depth;
    Bucket **direc = d->dir;
    int t = pow(2,gd);
    int h = k % t;
    Bucket *b = direc[h];
    Slot *s = b->slot1;
    if (s->chave == k){
        returned.b = h;
        returned.s = 1;
        return returned;
    }
    s = b->slot2;
    if (s->chave == k){
        returned.b = h;
        returned.s = 2;
        return returned;
    }
    return returned;
}

//Exclusão de entrada
Ret remover(Directory *d, int64_t k){
    Ret returned = buscar(d,k);
    Bucket **direc = d->dir;
    if (returned.b != (-1)){
        Bucket *b = direc[returned.b];
        if (returned.s == 1){
            b->slot1->chave = INT64_MAX;
            b->slot1->rid = INT64_MAX;
        }
        if (returned.s == 2){
            b->slot1->chave = INT64_MAX;
            b->slot1->rid = INT64_MAX;
        }
    }
    return returned;
}


int main(){
    Directory *direc = createDir();
    Ret ret;
    int gd = direc->global_depth;
    int64_t seq = 1;
    int64_t value;
    char command[4];
    FILE *entrada = fopen("entrada.txt", "r");
    FILE *saida = fopen("saida.txt", "w");
    const unsigned max = 100000;
    char buffer[max];
    int read;
    while (fgets(buffer, max, entrada)){
        read = sscanf(buffer, "%4s%" SCNd64 "\n", command, &value);
        printf("%s%" PRId64 "\n", command, value);
        if (strcmp(command, "INC:") == 1){
            ret = includeCompleto(direc, value, seq);
            seq = seq + 1;
            
        }
    }
    //do {
    //    scanf(" %c", &command);
    //    scanf(" %" PRId64, &value);
    //    if (command == 'i'){
    //        ret = includeCompleto(direc, value, seq);
    //        printf("b %d s %d d %d \n", ret.b, ret.s, ret.d);
    //    }
    //    if (command == 'b'){
    //        ret = buscar(direc, value);
    //        printf("b %d s %d \n", ret.b, ret.s);
    //    }
    //    if (command == 'r'){
    //        ret = remover(direc, value);
    //        printf("b %d s %d \n", ret.b, ret.s);
    //    }
    //} while (command != 's');
}