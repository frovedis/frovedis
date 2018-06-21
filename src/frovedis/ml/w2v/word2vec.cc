/*
 * The code is developed based on the below implementations.
 * https://code.google.com/archive/p/word2vec/
 * https://github.com/IntelLabs/pWord2Vec/
 */

#include "word2vec.hpp" 

#include <cstring>
#include <cmath>
#include <algorithm>
#include <unistd.h>
#include <mpi.h>

#ifdef _OPENMP
#include <omp.h>
#endif

// #ifdef USE_MKL
// #include "mkl.h"
// #endif
#include <cblas.h>

#include "../../matrix/rowmajor_matrix.hpp"
#include "../../core/serializehelper.hpp"
#include "../../core/log.hpp"


namespace frovedis {

namespace {

using namespace std;

#define MAX_STRING 100
#define EXP_TABLE_SIZE 1000
#define MAX_EXP 6
#define MAX_SENTENCE_LENGTH 1000
#define MPI_SCALAR MPI_FLOAT
// #define USE_MKL USE_BLAS
#define USE_BLAS 1

typedef float real;
typedef unsigned int uint;
// typedef unsigned long long ulonglong;
typedef size_t ulonglong;

struct vocab_word {
    uint cn;
    char *word;
};

class sequence {
public:
    int *indices;
    int *meta;
    int length;

    sequence(int len) {
        length = len;
        indices = (int *) malloc(length * sizeof(int));
        meta = (int *) malloc(length * sizeof(int));
    }
    ~sequence() {
        free(indices);
        free(meta);
    }
};

class train_config {
public:
  int hidden_size;
  int window;
  real sample;
  int negative;
  int iter;
  int min_count;
  real alpha;
  int batch_size;
  real model_sync_period;
  int min_sync_words;
  int full_sync_times;
  int message_size;
  int num_threads;

  SERIALIZE(hidden_size, window,
  sample, negative, iter, min_count,
  alpha, batch_size, model_sync_period, min_sync_words,
  full_sync_times, message_size, num_threads);
};

struct vocab_word *vocab = NULL;
int *vocab_hash = NULL;
int *vocab_count = NULL, *proc_train_data = NULL;   // Introduced to use the arguments 
int *table = NULL;
real *Wih = NULL, *Woh = NULL, *expTable = NULL;
 
int binary = 0, debug_mode = 2;
// bool disk = false;
int num_procs = 1, num_threads = 12, negative = 5, min_count = 5, min_reduce = 1, iter = 5, window = 5, batch_size = 11, my_rank = -1;
int vocab_max_size = 1000, vocab_size = 0, hidden_size = 100, min_sync_words = 1024, full_sync_times = 0;
int message_size = 1024; // MB
ulonglong train_words = 0, file_size = 0;
ulonglong proc_train_words = 0;
real alpha = 0.1f, sample = 1e-3f;
real model_sync_period = 0.1f;
const real EXP_RESOLUTION = EXP_TABLE_SIZE / (MAX_EXP * 2.0f);

char train_file[MAX_STRING], output_file[MAX_STRING];
char save_vocab_file[MAX_STRING], read_vocab_file[MAX_STRING];
const int vocab_hash_size = 30000000;  // Maximum 30 * 0.7 = 21M words in the vocabulary
const int table_size = 1e8;


void InitUnigramTable() {
    table = (int *) malloc(table_size * sizeof(int));

    const real power = 0.75f;
    double train_words_pow = 0.;
    #pragma omp parallel for num_threads(num_threads) reduction(+: train_words_pow)
    for (int i = 0; i < vocab_size; i++) {
        train_words_pow += pow(vocab_count[i], power);
        // train_words_pow += pow(vocab[i].cn, power);
    }

    int i = 0;
    // real d1 = pow(vocab[i].cn, power) / train_words_pow;
    real d1 = pow(vocab_count[i], power) / train_words_pow;
    for (int a = 0; a < table_size; a++) {
        table[a] = i;
        if (a / (real) table_size > d1) {
            i++;
	    if (i >= vocab_size) i = vocab_size - 1;
            // d1 += pow(vocab[i].cn, power) / train_words_pow;
            d1 += pow(vocab_count[i], power) / train_words_pow;
        }
        // if (i >= vocab_size)
        //    i = vocab_size - 1;
    }
}

// Reads a single word from a file, assuming space + tab + EOL to be word boundaries
void ReadWord(char *word, FILE *fin) {
    int a = 0, ch;
    while (!feof(fin)) {
        ch = fgetc(fin);
        if (ch == 13)
            continue;
        if ((ch == ' ') || (ch == '\t') || (ch == '\n')) {
            if (a > 0) {
                if (ch == '\n')
                    ungetc(ch, fin);
                break;
            }
            if (ch == '\n') {
                strcpy(word, (char *) "</s>");
                return;
            } else
                continue;
        }
        word[a] = ch;
        a++;
        if (a >= MAX_STRING - 1)
            a--;   // Truncate too long words
    }
    word[a] = 0;
}

// Returns hash value of a word
int GetWordHash(char *word) {
    uint hash = 0;
    for (int i = 0; i < strlen(word); i++)
        hash = hash * 257 + word[i];
    hash = hash % vocab_hash_size;
    return hash;
}

// Returns position of a word in the vocabulary; if the word is not found, returns -1
int SearchVocab(char *word) {
    int hash = GetWordHash(word);
    while (1) {
        if (vocab_hash[hash] == -1)
            return -1;
        if (!strcmp(word, vocab[vocab_hash[hash]].word))
            return vocab_hash[hash];
        hash = (hash + 1) % vocab_hash_size;
    }
    return -1;
}

// Reads a word and returns its index in the vocabulary
int ReadWordIndex(FILE *fin) {
    char word[MAX_STRING];
    ReadWord(word, fin);
    if (feof(fin))
        return -1;
    return SearchVocab(word);
}

// Adds a word to the vocabulary
int AddWordToVocab(char *word) {
    int hash, length = strlen(word) + 1;
    if (length > MAX_STRING)
        length = MAX_STRING;
    vocab[vocab_size].word = (char *) calloc(length, sizeof(char));
    strcpy(vocab[vocab_size].word, word);
    vocab[vocab_size].cn = 0;
    vocab_size++;
    // Reallocate memory if needed
    if (vocab_size + 2 >= vocab_max_size) {
        vocab_max_size += 1000;
        vocab = (struct vocab_word *) realloc(vocab, vocab_max_size * sizeof(struct vocab_word));
    }
    hash = GetWordHash(word);
    while (vocab_hash[hash] != -1)
        hash = (hash + 1) % vocab_hash_size;
    vocab_hash[hash] = vocab_size - 1;
    return vocab_size - 1;
}

// Used later for sorting by word counts
int VocabCompare(const void *a, const void *b) {
    return ((struct vocab_word *) b)->cn - ((struct vocab_word *) a)->cn;
}

// Sorts the vocabulary by frequency using word counts
void SortVocab() {
    // Sort the vocabulary and keep </s> at the first position
    qsort(&vocab[1], vocab_size - 1, sizeof(struct vocab_word), VocabCompare);
    memset(vocab_hash, -1, vocab_hash_size * sizeof(int));

    int size = vocab_size;
    train_words = 0;
    for (int i = 0; i < size; i++) {
        // Words occuring less than min_count times will be discarded from the vocab
        if ((vocab[i].cn < min_count) && (i != 0)) {
            vocab_size--;
            free(vocab[i].word);
        } else {
            // Hash will be re-computed, as after the sorting it is not actual
            int hash = GetWordHash(vocab[i].word);
            while (vocab_hash[hash] != -1)
                hash = (hash + 1) % vocab_hash_size;
            vocab_hash[hash] = i;
            train_words += vocab[i].cn;
        }
    }
    vocab = (struct vocab_word *) realloc(vocab, vocab_size * sizeof(struct vocab_word));
}

// Reduces the vocabulary by removing infrequent tokens
void ReduceVocab() {
    int count = 0;
    for (int i = 0; i < vocab_size; i++) {
        if (vocab[i].cn > min_reduce) {
            vocab[count].cn = vocab[i].cn;
            vocab[count].word = vocab[i].word;
            count++;
        } else {
            free(vocab[i].word);
        }
    }
    vocab_size = count;
    memset(vocab_hash, -1, vocab_hash_size * sizeof(int));

    for (int i = 0; i < vocab_size; i++) {
        // Hash will be re-computed, as it is not actual
        int hash = GetWordHash(vocab[i].word);
        while (vocab_hash[hash] != -1)
            hash = (hash + 1) % vocab_hash_size;
        vocab_hash[hash] = i;
    }
    min_reduce++;
}

void LearnVocabFromTrainFile() {
    char word[MAX_STRING];

    memset(vocab_hash, -1, vocab_hash_size * sizeof(int));

    FILE *fin = fopen(train_file, "rb");
    if (fin == NULL) {
        printf("ERROR: training data file not found!\n");
        exit(1);
    }

    train_words = 0;
    vocab_size = 0;
    AddWordToVocab((char *) "</s>");
    while (1) {
        ReadWord(word, fin);
        if (feof(fin))
            break;
        train_words++;
        if (my_rank == 0 && debug_mode > 1 && (train_words % 100000 == 0) && get_loglevel() <= DEBUG) {
            printf("%lldK%c", train_words / 1000, 13);
            fflush(stdout);
        }
        int i = SearchVocab(word);
        if (i == -1) {
            int a = AddWordToVocab(word);
            vocab[a].cn = 1;
        } else
            vocab[i].cn++;
        if (vocab_size > vocab_hash_size * 0.7)
            ReduceVocab();
    }
    SortVocab();
    if (my_rank == 0 && debug_mode > 0 && get_loglevel() <= DEBUG) {
        printf("Vocab size: %d\n", vocab_size);
        printf("Words in train file: %lld\n", train_words);
    }
    file_size = ftell(fin);
    fclose(fin);
}

void SaveVocab() {
    FILE *fo = fopen(save_vocab_file, "wb");
    for (int i = 0; i < vocab_size; i++)
        fprintf(fo, "%s %d\n", vocab[i].word, vocab[i].cn);
    fclose(fo);
}

void ReadVocab() {
    char word[MAX_STRING];
    FILE *fin = fopen(read_vocab_file, "rb");
    if (fin == NULL) {
        printf("Vocabulary file not found\n");
        exit(1);
    }
    memset(vocab_hash, -1, vocab_hash_size * sizeof(int));

    char c;
    vocab_size = 0;
    while (1) {
        ReadWord(word, fin);
        if (feof(fin))
            break;
        int i = AddWordToVocab(word);
        fscanf(fin, "%d%c", &vocab[i].cn, &c);
    }
    SortVocab();
    if (debug_mode > 0 && my_rank == 0 && get_loglevel() <= DEBUG) {
        printf("Vocab size: %d\n", vocab_size);
        printf("Words in train file: %lld\n", train_words);
    }
    fclose(fin);

    // get file size
    // FILE *fin2 = fopen(train_file, "rb");
    // if (fin2 == NULL) {
    //     printf("ERROR: training data file not found!\n");
    //     exit(1);
    // }
    // fseek(fin2, 0, SEEK_END);
    // file_size = ftell(fin2);
    // fclose(fin2);
}

void InitNet() {
    Wih = (real *) malloc(vocab_size * hidden_size * sizeof(real));
    Woh = (real *) malloc(vocab_size * hidden_size * sizeof(real));
    if (!Wih || !Woh) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    #pragma omp parallel for num_threads(num_threads) schedule(static, 1)
    for (int i = 0; i < vocab_size; i++) {
        memset(Wih + i * hidden_size, 0.f, hidden_size * sizeof(real));
        memset(Woh + i * hidden_size, 0.f, hidden_size * sizeof(real));
    }

    // initialization
    ulonglong next_random = 1;
    for (int i = 0; i < vocab_size * hidden_size; i++) {
        next_random = next_random * (ulonglong) 25214903917 + 11;
        Wih[i] = (((next_random & 0xFFFF) / 65536.f) - 0.5f) / hidden_size;
    }
}

ulonglong loadStream(FILE *fin, int *stream, const ulonglong total_words) {
    ulonglong word_count = 0;
    while (!feof(fin) && word_count < total_words) {
        int w = ReadWordIndex(fin);
        if (w == -1)
            continue;
        stream[word_count] = w;
        word_count++;
    }
    stream[word_count] = 0; // set the last word as "</s>"
    return word_count;
}

// assume v > 0
inline unsigned int getNumZeros(unsigned int v) {
    unsigned int numzeros = 0;
    while (!(v & 0x1)) {
        numzeros++;
        v = v >> 1;
    }
    return numzeros;
}

void Train_SGNS_MPI() {

// #ifdef USE_MKL
//     mkl_set_num_threads(1);
// #endif

    // if (read_vocab_file[0] != 0) {
    //     ReadVocab();
    // }
    // else {
    //     LearnVocabFromTrainFile();
    // }
    // if (my_rank == 0 && save_vocab_file[0] != 0) SaveVocab();
    // if (output_file[0] == 0) return;

    InitNet();
    InitUnigramTable();

    int num_parts = num_procs * (num_threads - 1);

    real starting_alpha = alpha;
    ulonglong word_count_actual = 0;

    int ready_threads = 0;
    int active_threads = num_threads - 1;
    bool compute_go = true;

    // LOG(DEBUG) << "start parallel threads at process " << my_rank << std::endl;
    
    #pragma omp parallel num_threads(num_threads)
    {

#ifdef _OPENMP
        int id = omp_get_thread_num();
#else
	int id = 0;
#endif

        if (id == 0) {
            // LOG(DEBUG) << "start process, thread: " << my_rank << ", " << id << std::endl; 
          
            int active_processes = 1;
            int active_processes_global = num_procs;
            ulonglong word_count_actual_global = 0;
            int sync_chunk_size = message_size * 1024 * 1024 / (hidden_size * sizeof(real));
            int full_sync_count = 1;
            unsigned int num_syncs = 0;

            while (ready_threads < num_threads - 1) {
                usleep(1);
            }
            MPI_Barrier(MPI_COMM_WORLD);

            // printf("ready_thread before imcrement at id : %d %d \n", ready_threads, id);
            // #pragma omp atomic  // TODO: check to remove
            ready_threads++;
            // printf("ready_thread after imcrement at id : %d %d \n", ready_threads, id);

#ifdef _OPENMP
            double start = omp_get_wtime();
#else
	    double start = get_dtime();
#endif
            double sync_start = start;

            while (1) {
                
#ifdef _OPENMP
                double sync_eclipsed = omp_get_wtime() - sync_start;
#else
                double sync_eclipsed = get_dtime() - sync_start;
#endif
                if (sync_eclipsed > model_sync_period) {
                    compute_go = false;
                    num_syncs++;
                    active_processes = (active_threads > 0 ? 1 : 0);

                    // synchronize parameters
                    MPI_Allreduce(&active_processes, &active_processes_global, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
                    MPI_Allreduce(&word_count_actual, &word_count_actual_global, 1, MPI_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);

                    // determine if full sync
                    int sync_vocab_size = min((1 << getNumZeros(num_syncs)) * min_sync_words, vocab_size);
                    real progress = word_count_actual_global / (real) (iter * train_words + 1);
                    if ((full_sync_times > 0) && (progress > (real)full_sync_count / (full_sync_times + 1) + 0.01f)) {
                        full_sync_count++;
                        sync_vocab_size = vocab_size;
                    }

                    int num_rounds = sync_vocab_size / sync_chunk_size + ((sync_vocab_size % sync_chunk_size > 0) ? 1 : 0);
                    for (int r = 0; r < num_rounds; r++) {
                        int start = r * sync_chunk_size;
                        int sync_size = min(sync_chunk_size, sync_vocab_size - start);
                        MPI_Allreduce(MPI_IN_PLACE, Wih + start * hidden_size, sync_size * hidden_size, MPI_SCALAR, MPI_SUM, MPI_COMM_WORLD);
                        MPI_Allreduce(MPI_IN_PLACE, Woh + start * hidden_size, sync_size * hidden_size, MPI_SCALAR, MPI_SUM, MPI_COMM_WORLD);                          
                    }

                    //#pragma simd
                    //#pragma vector aligned
                    for (int i = 0; i < sync_vocab_size * hidden_size; i++) {
                        Wih[i] /= num_procs;
                        Woh[i] /= num_procs;
                    }

                    // let it go!
                    compute_go = true;

                    // print out status
                    if (my_rank == 0 && debug_mode > 1 && get_loglevel() <= DEBUG) {
#ifdef _OPENMP
                        double now = omp_get_wtime();
#else
                        double now = get_dtime();
#endif
                        printf("%cAlpha: %f  Progress: %.2f%%  Words/sec: %.2fk  Words sync'ed: %d", 13, alpha,
                                progress * 100,
                                word_count_actual_global / ((now - start) * 1000),
                                sync_vocab_size);
                        fflush(stdout);
                    }

                    if (active_processes_global == 0) break;
#ifdef _OPENMP
                    sync_start = omp_get_wtime();
#else
                    sync_start = get_dtime();
#endif
                } else {
                    usleep(1);
                }
            }
        } else {  // id != 0
            // LOG(DEBUG) << "start process, thread: " << my_rank << ", " << id << std::endl;

            int local_iter = iter;
            ulonglong next_random = my_rank * (num_threads - 1) + id - 1;
            ulonglong word_count = 0, last_word_count = 0;
            int sentence_length = 0, sentence_position = 0;
            int sen[MAX_SENTENCE_LENGTH]; // __attribute__((aligned(64)));

            // load stream
            // FILE *fin = fopen(train_file, "rb");
            // ulonglong local_train_words = train_words / num_parts + (train_words % num_parts > 0 ? 1 : 0);
            ulonglong local_train_words = proc_train_words / (num_threads - 1) + (proc_train_words % (num_threads - 1) > 0 ? 1 : 0);
            int *stream;
            int w;
            // int offset = min(local_train_words * (my_rank * (num_threads - 1) + id - 1), train_words - 1);
            int offset = min(local_train_words * (id - 1), proc_train_words - 1);
            stream = proc_train_data + offset;
            local_train_words = min(local_train_words, proc_train_words - offset);
                        
            // if (!disk) {
            //     stream = (int *) malloc((local_train_words + 1) * sizeof(int));
            //     local_train_words = loadStream(fin, stream, local_train_words);
            //     fclose(fin);
            // }

            // printf("before allocate tmporary matrix - process, thread: %d, %d \n", my_rank, id);

            // temporary memory
            real * inputM = (real *) malloc(batch_size * hidden_size * sizeof(real));
            real * outputM = (real *) malloc((1 + negative) * hidden_size * sizeof(real));
            real * outputMd = (real *) malloc((1 + negative) * hidden_size * sizeof(real));
            real * corrM = (real *) malloc((1 + negative) * batch_size * sizeof(real));

            int inputs[2 * window + 1]; // __attribute__((aligned(64)));
            sequence outputs(1 + negative);

            // printf("ready_thread before imcrement at id : %d %d \n", ready_threads, id);
            // #pragma omp atomic  // NOTE: quick fix
            ready_threads++;
            // printf("ready_thread after imcrement at id : %d %d \n", ready_threads, id);

            while (ready_threads < num_threads) {
                usleep(1);
            }
            // printf("before while process, thread: %d, %d \n", my_rank, id);

            while (1) {
                
                while (!compute_go) {
                    usleep(1);
                }

                if (word_count - last_word_count > 10000) {
                    ulonglong diff = word_count - last_word_count;
                    #pragma omp atomic
                    word_count_actual += diff;
                    last_word_count = word_count;

                    // update alpha
                    alpha = starting_alpha * (1 - word_count_actual * num_procs / (real) (iter * train_words + 1));
                    if (alpha < starting_alpha * 0.0001f)
                        alpha = starting_alpha * 0.0001f;
                }
                if (sentence_length == 0) {
                    while (1) {
                        // if (disk) {
                        //     w = ReadWordIndex(fin);
                        //     if (feof(fin)) break;
                        //     if (w == -1) continue;
                        // } else {
                        //     w = stream[word_count];
                        // }
                        w = stream[word_count];
                        word_count++;
                        // if (w == 0) break;
                        if (word_count == local_train_words) break;
                        // The subsampling randomly discards frequent words while keeping the ranking same
                        if (sample > 0) {
                            // real ratio = (sample * train_words) / vocab[w].cn;
                            real ratio = (sample * train_words) / vocab_count[w];
                            real ran = sqrtf(ratio) + ratio;
                            next_random = next_random * (ulonglong) 25214903917 + 11;
                            if (ran < (next_random & 0xFFFF) / 65536.f)
                                continue;
                        }
                        sen[sentence_length] = w;
                        sentence_length++;
                        if (sentence_length >= MAX_SENTENCE_LENGTH) break;
                    }
                    sentence_position = 0;
                }
                // if ((disk && feof(fin)) || word_count > local_train_words) {
                if (word_count > local_train_words - 1) {
                  
                    ulonglong diff = word_count - last_word_count;
                    #pragma omp atomic
                    word_count_actual += diff;

                    local_iter--;
                    if (local_iter == 0) {
                        #pragma omp atomic
                        active_threads--;
                        break;
                    }
                    word_count = 0;
                    last_word_count = 0;
                    sentence_length = 0;
                    // if (disk) {
                    //     fseek(fin, file_size * (my_rank * (num_threads - 1) + id - 1) / num_parts, SEEK_SET);
                    // }
                    continue;
                }

                int target = sen[sentence_position];
                outputs.indices[0] = target;
                outputs.meta[0] = 1;

                // get all input contexts around the target word
                next_random = next_random * (ulonglong) 25214903917 + 11;
                int b = next_random % window;

                int num_inputs = 0;
                for (int i = b; i < 2 * window + 1 - b; i++) {
                    if (i != window) {
                        int c = sentence_position - window + i;
                        if (c < 0)
                            continue;
                        if (c >= sentence_length)
                            break;
                        inputs[num_inputs] = sen[c];
                        num_inputs++;
                    }
                }

                int num_batches = num_inputs / batch_size + ((num_inputs % batch_size > 0) ? 1 : 0);

                // start mini-batches
                for (int b = 0; b < num_batches; b++) {

                    // generate negative samples for output layer
                    int offset = 1;
                    for (int k = 0; k < negative; k++) {
                        next_random = next_random * (ulonglong) 25214903917 + 11;
                        int sample = table[(next_random >> 16) % table_size];
                        if (!sample)
                            sample = next_random % (vocab_size - 1) + 1;
                        int* p = find(outputs.indices, outputs.indices + offset, sample);
                        if (p == outputs.indices + offset) {
                            outputs.indices[offset] = sample;
                            outputs.meta[offset] = 1;
                            offset++;
                        } else {
                            int idx = p - outputs.indices;
                            outputs.meta[idx]++;
                        }
                    }
                    outputs.meta[0] = 1;
                    outputs.length = offset;

                    // fetch input sub model
                    int input_start = b * batch_size;
                    int input_size = min(batch_size, num_inputs - input_start);
                    for (int i = 0; i < input_size; i++) {
                        memcpy(inputM + i * hidden_size, Wih + inputs[input_start + i] * hidden_size, hidden_size * sizeof(real));
                    }
                    // fetch output sub model
                    int output_size = outputs.length;
                    for (int i = 0; i < output_size; i++) {
                        memcpy(outputM + i * hidden_size, Woh + outputs.indices[i] * hidden_size, hidden_size * sizeof(real));
                    }

#ifndef USE_BLAS
                    for (int i = 0; i < output_size; i++) {
                        int c = outputs.meta[i];
                        for (int j = 0; j < input_size; j++) {
                            real f = 0.f, g;
                            //#pragma simd
                            for (int k = 0; k < hidden_size; k++) {
                                f += outputM[i * hidden_size + k] * inputM[j * hidden_size + k];
                            }
                            int label = (i ? 0 : 1);
                            if (f > MAX_EXP)
                                g = (label - 1) * alpha;
                            else if (f < -MAX_EXP)
                                g = label * alpha;
                            else
                                g = (label - expTable[(int) ((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
                            corrM[i * input_size + j] = g * c;
                        }
                    }
#else
                    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasTrans, output_size, input_size, hidden_size, 1.0f, outputM,
                            hidden_size, inputM, hidden_size, 0.0f, corrM, input_size);
                    for (int i = 0; i < output_size; i++) {
                        int c = outputs.meta[i];
                        int offset = i * input_size;
                        //#pragma simd
                        for (int j = 0; j < input_size; j++) {
                            real f = corrM[offset + j];
                            int label = (i ? 0 : 1);
                            if (f > MAX_EXP)
                                f = (label - 1) * alpha;
                            else if (f < -MAX_EXP)
                                f = label * alpha;
                            else
                                f = (label - expTable[(int) ((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
                            corrM[offset + j] = f * c;
                        }
                    }
#endif

#ifndef USE_BLAS
                    for (int i = 0; i < output_size; i++) {
                        for (int j = 0; j < hidden_size; j++) {
                            real f = 0.f;
                            //#pragma simd
                            for (int k = 0; k < input_size; k++) {
                                f += corrM[i * input_size + k] * inputM[k * hidden_size + j];
                            }
                            outputMd[i * hidden_size + j] = f;
                        }
                    }
#else
                    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, output_size, hidden_size, input_size, 1.0f, corrM,
                            input_size, inputM, hidden_size, 0.0f, outputMd, hidden_size);
#endif

#ifndef USE_BLAS
                    for (int i = 0; i < input_size; i++) {
                        for (int j = 0; j < hidden_size; j++) {
                            real f = 0.f;
                            //#pragma simd
                            for (int k = 0; k < output_size; k++) {
                                f += corrM[k * input_size + i] * outputM[k * hidden_size + j];
                            }
                            inputM[i * hidden_size + j] = f;
                        }
                    }
#else
                    cblas_sgemm(CblasRowMajor, CblasTrans, CblasNoTrans, input_size, hidden_size, output_size, 1.0f, corrM,
                            input_size, outputM, hidden_size, 0.0f, inputM, hidden_size);
#endif

                    // subnet update
                    for (int i = 0; i < input_size; i++) {
                        int src = i * hidden_size;
                        int des = inputs[input_start + i] * hidden_size;
                        //#pragma simd
                        for (int j = 0; j < hidden_size; j++) {
                            Wih[des + j] += inputM[src + j];
                        }
                    }

                    for (int i = 0; i < output_size; i++) {
                        int src = i * hidden_size;
                        int des = outputs.indices[i] * hidden_size;
                        //#pragma simd
                        for (int j = 0; j < hidden_size; j++) {
                            Woh[des + j] += outputMd[src + j];
                        }
                    }

                }

                sentence_position++;
                if (sentence_position >= sentence_length) {
                    sentence_length = 0;
                }
            }
            free(inputM);
            free(outputM);
            free(outputMd);
            free(corrM);
            // if (disk) {
            //     fclose(fin);
            // } else {
            //     free(stream);
            // }
        }
    }
}

/*
int ArgPos(char *str, int argc, char **argv) {
    for (int a = 1; a < argc; a++)
        if (!strcmp(str, argv[a])) {
            //if (a == argc - 1) {
            //    printf("Argument missing for %s\n", str);
            //    exit(1);
            //}
            return a;
        }
    return -1;
}
*/

void saveModel() {
    // save the model
    FILE *fo = fopen(output_file, "wb");
    // Save the word vectors
    fprintf(fo, "%d %d\n", vocab_size, hidden_size);
    for (int a = 0; a < vocab_size; a++) {
        fprintf(fo, "%s ", vocab[a].word);
        if (binary)
            for (int b = 0; b < hidden_size; b++)
                fwrite(&Wih[a * hidden_size + b], sizeof(real), 1, fo);
        else
            for (int b = 0; b < hidden_size; b++)
                fprintf(fo, "%f ", Wih[a * hidden_size + b]);
        fprintf(fo, "\n");
    }
    fclose(fo);
}

rowmajor_matrix_local<real> w2v_train_local(
    std::vector<int>& _proc_train_data, 
    std::vector<int>& _vocab_count,
    train_config& _config) {
  
  hidden_size = _config.hidden_size;
  window = _config.window;
  sample = _config.sample;
  negative = _config.negative;
  iter = _config.iter;
  min_count = _config.min_count;
  alpha = _config.alpha;
  batch_size = _config.batch_size;
  model_sync_period = _config.model_sync_period;
  min_sync_words = _config.min_sync_words;
  full_sync_times = _config.full_sync_times;
  message_size = _config.message_size;
  num_threads = _config.num_threads;
  
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  proc_train_data = _proc_train_data.data();
  proc_train_words = _proc_train_data.size();
  MPI_Allreduce(&proc_train_words, &train_words, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  // vocab = (struct vocab_word *) calloc(vocab_max_size, sizeof(struct vocab_word));
  // vocab_hash = (int *) malloc(vocab_hash_size * sizeof(int));
  expTable = (real *) malloc((EXP_TABLE_SIZE + 1) * sizeof(real));
  for (int i = 0; i < EXP_TABLE_SIZE + 1; i++) {
      expTable[i] = exp((i / (real) EXP_TABLE_SIZE * 2 - 1) * MAX_EXP); // Precompute the exp() table
      expTable[i] = expTable[i] / (expTable[i] + 1);                    // Precompute f(x) = x / (x + 1)
  }
  // _vocab_count.resize(_vocab_count.size() + 1000);
  vocab_count = _vocab_count.data();
  vocab_size = _vocab_count.size();

  // LOG(DEBUG) << "train words: " << train_words << std::endl;
  // LOG(DEBUG) << "train_words / process: " << proc_train_words << std::endl;
  // LOG(DEBUG) << "vocab size: " << vocab_size << std::endl; 

  Train_SGNS_MPI();
  
  rowmajor_matrix_local<real> ret(vocab_size, hidden_size, Wih);
  
  free(Wih);
  free(Woh);
  // free(vocab);
  // free(vocab_hash);
  free(expTable);
  free(table);

  return ret;
}

}  // namespace "anonymous"

rowmajor_matrix_local<real> w2v_train(
    node_local<std::vector<int>>& _nl_proc_train_data, std::vector<int>& _vocab_count,
    int _hidden_size = 100, int _window = 5, real _sample = 1e-3f, int _negative = 5, int _iter = 5, int _min_count = 5,
    real _alpha = 0.1, int _batch_size = 11, real _model_sync_period = 0.1, int _min_sync_words = 1024,
    int _full_sync_times = 0, int _message_size = 1024, int _num_threads = 12) {
      
  train_config _config = {_hidden_size, _window,
    _sample, _negative, _iter, _min_count,
    _alpha, _batch_size, _model_sync_period, _min_sync_words,
    _full_sync_times, _message_size, _num_threads};

  return _nl_proc_train_data.map(w2v_train_local, broadcast(_vocab_count), broadcast(_config)).get(0);
}

void w2v_build_vocab_and_dump(const std::string& _train_file_str, const std::string& _stream_file_str,
                              const std::string& _vocab_file_str, const std::string& _vocab_count_file_str) {
                            
  vocab = (struct vocab_word *) calloc(vocab_max_size, sizeof(struct vocab_word));
  vocab_hash = (int *) malloc(vocab_hash_size * sizeof(int));
  
  strcpy(train_file, _train_file_str.c_str());
  LearnVocabFromTrainFile();
  
  strcpy(save_vocab_file, _vocab_file_str.c_str());
  SaveVocab();
  
  std::vector<int> vocab_count_vec(vocab_size, 0);
  for (size_t i = 0; i < vocab_size; i++) {
    vocab_count_vec[i] = vocab[i].cn;
  }
  make_dvector_scatter(vocab_count_vec).savebinary(_vocab_count_file_str);

  FILE *fin = fopen(train_file, "rb");
  std::vector<int> stream_vec(train_words + 1, 0);
  loadStream(fin, stream_vec.data(), train_words);
  fclose(fin);
  make_dvector_scatter(stream_vec).savebinary(_stream_file_str);
    
  for (int i = 0; i < vocab_size; i++)  free(vocab[i].word);
  free(vocab);
  free(vocab_hash);
}

void w2v_save_model(const rowmajor_matrix_local<real>& weight,
                    const std::string& _vocab_file_str,
                    const std::string& _output_file_str){
                      
  vocab = (struct vocab_word *) calloc(vocab_max_size, sizeof(struct vocab_word));
  vocab_hash = (int *) malloc(vocab_hash_size * sizeof(int));

  strcpy(read_vocab_file, _vocab_file_str.c_str());
  ReadVocab();

  strcpy(output_file, _output_file_str.c_str());
  // vocab_size = weight.local_num_row;
  hidden_size = weight.local_num_col;
  binary = 1;

  Wih = (real *) malloc(vocab_size * hidden_size * sizeof(real));
  for (size_t i = 0; i < vocab_size * hidden_size; i++) {
    Wih[i] = weight.val[i];
  }
  
  saveModel();
  
  free(Wih);
  for (int i = 0; i < vocab_size; i++)  free(vocab[i].word);
  free(vocab);
  free(vocab_hash);
}


}  // namespace frovedis

