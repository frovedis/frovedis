/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil ; -*- */
/*
 * Copyright 2018 NEC Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * The code is developed based on the below implementations.
 * https://code.google.com/archive/p/word2vec/
 * https://github.com/IntelLabs/pWord2Vec/
 */

#include "w2v_core.hpp"

#include <cstring>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <omp.h>
#include <sys/time.h>

#include "w2v_env.incl"

#include "w2vgemm.hpp"
#include "w2v_common.hpp"

#include "frovedis/core/mpi_rpc.hpp"

#ifdef W2V_USE_MPI
#include <mpi.h>
#endif

#ifdef W2V_USE_FTRACE
#include <ftrace.h>
#define FTRACE_REGION_BEGIN(ID) ftrace_region_begin(ID)
#define FTRACE_REGION_END(ID) ftrace_region_end(ID)
#else
#define FTRACE_REGION_BEGIN(ID)                                                \
  {}
#define FTRACE_REGION_END(ID)                                                  \
  {}
#endif

#define EXP_TABLE_SIZE 1000
#define MAX_EXP 6
#define MAX_SENTENCE_LENGTH 1000
#ifdef W2V_USE_MPI
#define MPI_SCALAR MPI_FLOAT
#endif

//#define W2V_DEBUG
#if defined(W2V_DEBUG) && defined(W2V_USE_MPI)
#define DEBUG(fmt, args...) fprintf(stderr, "%d:%s(): " fmt, __LINE__, __func__, ##args)
#else
#define DEBUG(fmt, args...) /* Don't do anything in release builds */
#endif

namespace frovedis {
  void init_shm_info_local();
}

namespace w2v {

using namespace std;

namespace {

int *vocab_count = NULL,
    *proc_train_data = NULL; // Introduced to use the arguments
int *table = NULL;
real *Wih = NULL, *Woh = NULL, *expTable = NULL;

//int binary = 0, debug_mode = 1;
int num_procs = 1, num_threads = 0, negative = 5, 
    iter = 5, window = 5, batch_size = 11, my_rank = -1;
int vocab_size = 0, hidden_size = 100,
    min_sync_words = 1024, full_sync_times = 0;
int message_size = 1024; // MB
ulonglong train_words = 0 /* , file_size = 0 */;
ulonglong proc_train_words = 0;
real alpha = 0.1f, sample = 1e-3f;
real model_sync_period = 0.1f;

const int table_size = 1e8;

#ifdef W2V_USE_INTRINSIC
#ifdef USE_COMBINED_GEMM_ALL
pFuncW2VKernel tableW2VKernel[] = {
  NULL,                       w2v_kernel_N6_HU512_2X_W01,
  w2v_kernel_N6_HU512_2X_W02, w2v_kernel_N6_HU512_2X_W03,
  w2v_kernel_N6_HU512_2X_W04, w2v_kernel_N6_HU512_2X_W05,
  w2v_kernel_N6_HU512_2X_W06, w2v_kernel_N6_HU512_2X_W07,
  w2v_kernel_N6_HU512_2X_W08, w2v_kernel_N6_HU512_2X_W09,
  w2v_kernel_N6_HU512_2X_W10, w2v_kernel_N6_HU512_2X_W11,
  w2v_kernel_N6_HU512_2X_W12, w2v_kernel_N6_HU512_2X_W13,
  w2v_kernel_N6_HU512_2X_W14, w2v_kernel_N6_HU512_2X_W15,
  w2v_kernel_N6_HU512_2X_W16, NULL
};
#endif

#if defined(USE_COMBINED_GEMM_12) || defined(USE_COMBINED_GEMM_EACH)
pFuncVGemm0 tableVGemm0[] = {
  NULL,                   w2v_gemm0_HU512_2X_W01, w2v_gemm0_HU512_2X_W02,
  w2v_gemm0_HU512_2X_W03, w2v_gemm0_HU512_2X_W04, w2v_gemm0_HU512_2X_W05,
  w2v_gemm0_HU512_2X_W06, w2v_gemm0_HU512_2X_W07, w2v_gemm0_HU512_2X_W08,
  w2v_gemm0_HU512_2X_W09, w2v_gemm0_HU512_2X_W10, w2v_gemm0_HU512_2X_W11,
  w2v_gemm0_HU512_2X_W12, w2v_gemm0_HU512_2X_W13, w2v_gemm0_HU512_2X_W14,
  w2v_gemm0_HU512_2X_W15, w2v_gemm0_HU512_2X_W16, NULL
};
#endif

#if defined(USE_COMBINED_GEMM_12)
pFuncVGemm12 tableVGemm12[] = {
  NULL,                       w2v_gemm12_N6_HU512_2X_W01,
  w2v_gemm12_N6_HU512_2X_W02, w2v_gemm12_N6_HU512_2X_W03,
  w2v_gemm12_N6_HU512_2X_W04, w2v_gemm12_N6_HU512_2X_W05,
  w2v_gemm12_N6_HU512_2X_W06, w2v_gemm12_N6_HU512_2X_W07,
  w2v_gemm12_N6_HU512_2X_W08, w2v_gemm12_N6_HU512_2X_W09,
  w2v_gemm12_N6_HU512_2X_W10, w2v_gemm12_N6_HU512_2X_W11,
  w2v_gemm12_N6_HU512_2X_W12, w2v_gemm12_N6_HU512_2X_W13,
  w2v_gemm12_N6_HU512_2X_W14, w2v_gemm12_N6_HU512_2X_W15,
  w2v_gemm12_N6_HU512_2X_W16, NULL
};
#elif defined(USE_COMBINED_GEMM_EACH)
pFuncVGemm2 tableVGemm2HU256[] = {
  NULL,                w2v_gemm2_HU256_W01, w2v_gemm2_HU256_W02,
  w2v_gemm2_HU256_W03, w2v_gemm2_HU256_W04, w2v_gemm2_HU256_W05,
  w2v_gemm2_HU256_W06, w2v_gemm2_HU256_W07, w2v_gemm2_HU256_W08,
  w2v_gemm2_HU256_W09, w2v_gemm2_HU256_W10, w2v_gemm2_HU256_W11,
  w2v_gemm2_HU256_W12, w2v_gemm2_HU256_W13, w2v_gemm2_HU256_W14,
  w2v_gemm2_HU256_W15, w2v_gemm2_HU256_W16, NULL
};
pFuncVGemm2 tableVGemm2HU512[] = {
  NULL,                   w2v_gemm2_HU512_2X_W01, w2v_gemm2_HU512_2X_W02,
  w2v_gemm2_HU512_2X_W03, w2v_gemm2_HU512_2X_W04, w2v_gemm2_HU512_2X_W05,
  w2v_gemm2_HU512_2X_W06, w2v_gemm2_HU512_2X_W07, w2v_gemm2_HU512_2X_W08,
  w2v_gemm2_HU512_2X_W09, w2v_gemm2_HU512_2X_W10, w2v_gemm2_HU512_2X_W11,
  w2v_gemm2_HU512_2X_W12, w2v_gemm2_HU512_2X_W13, w2v_gemm2_HU512_2X_W14,
  w2v_gemm2_HU512_2X_W15, w2v_gemm2_HU512_2X_W16, NULL
};
#endif
#endif // W2V_USE_INTRINSIC


inline double get_dtime() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  return ((double)(tv.tv_sec) + (double)(tv.tv_usec) * 0.001 * 0.001);
}

class sequence {
public:
  int *indices;
  int *meta;
  int length;

  sequence(int len) {
    length = len;
    indices = (int *)malloc(length * sizeof(int));
    meta = (int *)malloc(length * sizeof(int));
  }
  ~sequence() {
    free(indices);
    free(meta);
  }
};

void InitUnigramTable() {
  table = (int *)malloc(table_size * sizeof(int));

  const real power = 0.75f;
  double train_words_pow = 0.;
#pragma omp parallel for num_threads(num_threads) reduction(+ : train_words_pow)
  for (int i = 0; i < vocab_size; i++) {
    train_words_pow += pow(vocab_count[i], power);
  }

  int i = 0;
  real d1 = pow(vocab_count[i], power) / train_words_pow;
  for (int a = 0; a < table_size; a++) {
    table[a] = i;
    if (a / (real)table_size > d1) {
      i++;
      if (i >= vocab_size)
        i = vocab_size - 1;
      d1 += pow(vocab_count[i], power) / train_words_pow;
    }
  }
}

void InitNet() {
  Wih = (real *)malloc(vocab_size * hidden_size * sizeof(real));
  Woh = (real *)malloc(vocab_size * hidden_size * sizeof(real));
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
  real hidden65536 = hidden_size * (real)65536;
  for (int i = 0; i < vocab_size * hidden_size; i++) {
    next_random = next_random * (ulonglong)25214903917 + 11;
    Wih[i] = (int(next_random & 0xFFFF) - 65536 / 2) / hidden65536;
  }
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


void Train_SGNS_general() {

  InitNet();
  InitUnigramTable();

  real starting_alpha = alpha;
  ulonglong word_count_actual = 0;

#ifdef W2V_USE_MPI
  int ready_threads = 0;
  int active_threads = num_threads - 1;
  bool compute_go = true;
#endif

#pragma omp parallel num_threads(num_threads)
  {


    int id = omp_get_thread_num();

#ifdef W2V_USE_MPI
    if (id == 0) {

      int active_processes = 1;
      int active_processes_global = num_procs;
      ulonglong word_count_actual_global = 0;
      int sync_chunk_size =
          message_size * 1024 * 1024 / (hidden_size * sizeof(real));
      int full_sync_count = 1;
      unsigned int num_syncs = 0;

      while (ready_threads < num_threads - 1) {
        usleep(1);
      }
      MPI_Barrier(frovedis::frovedis_comm_rpc);

#pragma omp atomic
      ready_threads = ready_threads + 1;

      double start = get_dtime();
      double sync_start = start;

      while (1) {

        double sync_eclipsed = get_dtime() - sync_start;

        if (sync_eclipsed > model_sync_period) {

          compute_go = false;
          num_syncs++;
          active_processes = (active_threads > 0 ? 1 : 0);

          // synchronize parameters
          MPI_Allreduce(&active_processes, &active_processes_global, 1, MPI_INT,
                        MPI_SUM, frovedis::frovedis_comm_rpc);
          MPI_Allreduce(&word_count_actual, &word_count_actual_global, 1,
                        MPI_LONG_LONG, MPI_SUM, frovedis::frovedis_comm_rpc);

          // determine if full sync
          int sync_vocab_size =
              min((1 << getNumZeros(num_syncs)) * min_sync_words, vocab_size);
          real progress =
              word_count_actual_global / (real)(iter * train_words + 1);
          if ((full_sync_times > 0) &&
              (progress >
               (real)full_sync_count / (full_sync_times + 1) + 0.01f)) {
            full_sync_count++;
            sync_vocab_size = vocab_size;
          }

          int num_rounds = sync_vocab_size / sync_chunk_size +
                           ((sync_vocab_size % sync_chunk_size > 0) ? 1 : 0);
          for (int r = 0; r < num_rounds; r++) {
            int start = r * sync_chunk_size;
            int sync_size = min(sync_chunk_size, sync_vocab_size - start);
            MPI_Allreduce(MPI_IN_PLACE, Wih + start * hidden_size,
                          sync_size * hidden_size, MPI_SCALAR, MPI_SUM,
                          frovedis::frovedis_comm_rpc);
            MPI_Allreduce(MPI_IN_PLACE, Woh + start * hidden_size,
                          sync_size * hidden_size, MPI_SCALAR, MPI_SUM,
                          frovedis::frovedis_comm_rpc);
          }

          for (int i = 0; i < sync_vocab_size * hidden_size; i++) {
            Wih[i] /= num_procs;
            Woh[i] /= num_procs;
          }

          // let it go!
          compute_go = true;

          if (active_processes_global == 0)
            break;
          sync_start = get_dtime();
        } else {
          usleep(1);
        }
      }

    } else { // if(id)

#endif       // W2V_USE_MPI

      int local_iter = iter;
      // ulonglong next_random = id;
      ulonglong next_random = my_rank * (num_threads - 1) + id - 1;
      ulonglong word_count = 0, last_word_count = 0;
      int sentence_length = 0, sentence_position = 0;
#ifdef W2V_USE_BUNCH_ITER
#define BUNCH_ITER 256
      int *pSenBase = (int *)malloc(
          (window + MAX_SENTENCE_LENGTH * BUNCH_ITER + BUNCH_ITER + window) *
          sizeof(int));
      int *sen = &pSenBase[window];
#else
      int *pSenBase =
        (int *)malloc((window + MAX_SENTENCE_LENGTH + window) * sizeof(int));
      int *sen = &pSenBase[window];
#endif

#ifdef W2V_USE_MPI
      int compute_threads = num_threads - 1;
      int compute_id = id - 1;
#else
      int compute_threads = num_threads;
      int compute_id = id;
#endif

      ulonglong local_train_words =
          proc_train_words / compute_threads +
          (proc_train_words % compute_threads > 0 ? 1 : 0);
      int *stream;
      int stream_offset =
          min(local_train_words * compute_id, proc_train_words - 1);
      stream = proc_train_data + stream_offset;
      local_train_words =
          min(local_train_words, proc_train_words - stream_offset);

#if defined(USE_COMBINED_GEMM_ALL)
      int are_supposed_parameters =      \
          hidden_size <= 512 &&    \
          hidden_size % 2 == 0 &&  \
          negative == 5 &&         \
          window <= 8;
#endif

// temporary memory
      real *inputM, *outputM, *outputMd, *corrM;
#if !defined(USE_COMBINED_GEMM_ALL) && !defined(USE_COMBINED_GEMM_12) &&       \
    !defined(USE_COMBINED_GEMM_EACH)
      inputM = (real *)malloc(batch_size * hidden_size * sizeof(real));
      outputM =
          (real *)malloc((1 + negative) * hidden_size * sizeof(real));
#endif
#if !defined(USE_COMBINED_GEMM_ALL) && !defined(USE_COMBINED_GEMM_12)
      outputMd =
          (real *)malloc((1 + negative) * hidden_size * sizeof(real));
#elif defined(USE_COMBINED_GEMM_12)
      outputMd = NULL;
#endif
#if defined(USE_COMBINED_GEMM_ALL)
      if (!are_supposed_parameters) {
        inputM = (real *)malloc(batch_size * hidden_size * sizeof(real));
        outputM =
            (real *)malloc((1 + negative) * hidden_size * sizeof(real));
        outputMd =
            (real *)malloc((1 + negative) * hidden_size * sizeof(real));
      }
#endif
      corrM = (real *)malloc((1 + negative) * batch_size * sizeof(real));

#ifdef W2V_USE_BUNCH_ITER
      int *bn_inputs = (int *)malloc(batch_size * BUNCH_ITER * sizeof(int));
      int *bn_outputs =
          (int *)malloc((1 + negative) * BUNCH_ITER * sizeof(int));
      int *bn_num_inputs = (int *)malloc(BUNCH_ITER * sizeof(int));
      int *bn_b = (int *)malloc(BUNCH_ITER * sizeof(int));
#else
      int inputs[2 * window + 1]; // __attribute__((aligned(64)));
      sequence outputs(1 + negative);
#endif

#ifdef W2V_USE_MPI
#pragma omp atomic
      ready_threads = ready_threads + 1;
      while (ready_threads < num_threads) {
        usleep(1);
      }
#else
#pragma omp barrier
#endif  // W2V_USE_MPI


      while (1) {
#ifdef W2V_USE_MPI
        while (!compute_go) {
          usleep(1);
        }
#endif

        if (word_count - last_word_count > 10000) {
          ulonglong diff = word_count - last_word_count;
#pragma omp atomic
          word_count_actual += diff;
          last_word_count = word_count;

          alpha = starting_alpha *
                  (1 - word_count_actual * num_procs / (real)(iter * train_words + 1));
          if (alpha < starting_alpha * 0.0001f)
            alpha = starting_alpha * 0.0001f;
        }

        if (sentence_length == 0 && word_count < local_train_words) {
#ifdef W2V_USE_BUNCH_ITER
          int read_length = min(ulonglong(MAX_SENTENCE_LENGTH * BUNCH_ITER),
                                local_train_words - word_count);
#else
          int read_length =
            min(ulonglong(MAX_SENTENCE_LENGTH), local_train_words - word_count);
#endif
          next_random = next_random * (ulonglong)25214903917 + 11;
          for (int i = 0; i < read_length; i++) {
            int w = stream[word_count + i];
            real ratio = (sample * train_words) / vocab_count[w];
            real ran = sqrtf(ratio) + ratio;
            ulonglong tmp_random =
                (next_random + i + 1) * (ulonglong)(25214903917 + i) +
                11; // VECTORIZED RANDOM
            int accept_or_iszero =
                (ran * (real)65536 > (tmp_random & 0xFFFF) || w == 0);
            if (accept_or_iszero) {
              sen[sentence_length] = w;
              sentence_length++;
            }
          }
          word_count += read_length;

          sentence_position = 0;
        }

        if (word_count > local_train_words - 1 && sentence_length == 0) {

          ulonglong diff = word_count - last_word_count;
#pragma omp atomic
          word_count_actual += diff;

          local_iter--;
          if (local_iter == 0) {
#ifdef W2V_USE_MPI
#pragma omp atomic
            active_threads--;
#endif
            break;
          }
          word_count = 0;
          last_word_count = 0;
          sentence_length = 0;
          continue;
        }

#ifdef W2V_USE_BUNCH_ITER
        // bunch get input context
        next_random = next_random * (ulonglong)25214903917 + 11;
        int bn_left_out[BUNCH_ITER];
        int bn_right_out[BUNCH_ITER];
#if BUNCH_ITER <= 256
#pragma _NEC shortloop
#endif
        for (int ib = 0; ib < BUNCH_ITER; ib++) {
          ulonglong tmp_random =
              (next_random + ib + 1) * (ulonglong)(25214903917 + ib) +
              11; // VECTORIZED RANDOM
          bn_b[ib] = (tmp_random >> 16) % window;
          bn_num_inputs[ib] = 0;
          bn_left_out[ib] = 0;
          bn_right_out[ib] = 0;
        }
        for (int i = 0; i < window; i++) {
#if BUNCH_ITER <= 256
#pragma _NEC shortloop
#endif
#pragma _NEC ivdep
          for (int ib = 0; ib < BUNCH_ITER; ib++) {
            int tmp = sentence_position + ib;
            int left_c = tmp - (i + 1);
            int right_c = tmp + (i + 1);
            int tmp_bn_num_inputs = bn_num_inputs[ib];
            int w_left = sen[left_c];
            int w_right = sen[right_c];
            int left_out = bn_left_out[ib];
            int right_out = bn_right_out[ib];
            if (w_left == 0) {
              bn_left_out[ib] = left_out = 1;
            }
            if (w_right == 0) {
              bn_right_out[ib] = right_out = 1;
            }
            if (i <= bn_b[ib]) {
              if (left_c >= 0) {
                if (left_c < sentence_length) {
                  if (!left_out) {
                    bn_inputs[ib * batch_size + tmp_bn_num_inputs] =
                        sen[left_c];
                    tmp_bn_num_inputs++;
                  }
                }
              }
              if (right_c < sentence_length) {
                if (!right_out) {
                  bn_inputs[ib * batch_size + tmp_bn_num_inputs] = sen[right_c];
                  tmp_bn_num_inputs++;
                }
              }
            }
            bn_num_inputs[ib] = tmp_bn_num_inputs;
          }
        }
#else  // W2V_USE_BUNCH_ITER
        int target = sen[sentence_position];
        outputs.indices[0] = target;
        outputs.meta[0] = 1;

        // get all input contexts around the target word
        next_random = next_random * (ulonglong)25214903917 + 11;
        int b = next_random % window;

        int num_inputs = 0;
        int left_out = 0;
        int right_out = 0;
        for (int i = 0; i < window - b; i++) {
          int left_c = sentence_position - (i + 1);
          int right_c = sentence_position + (i + 1);
          int w_left = sen[left_c];
          int w_right = sen[right_c];
          if (w_left == 0)
            left_out = 1;
          if (w_right == 0)
            right_out = 1;
          if (left_c >= 0) {
            if (left_c < sentence_length) {
              if (!left_out) {
                inputs[num_inputs] = w_left;
                num_inputs++;
              }
            }
          }
          if (right_c < sentence_length) {
            if (!right_out) {
              inputs[num_inputs] = w_right;
              num_inputs++;
            }
          }
        }
#endif // W2V_USE_BUNCH_ITER

#ifdef W2V_USE_BUNCH_ITER
        // bunch generate negative
        next_random = next_random * (ulonglong)25214903917 + 11;
        for (int ibk = 0; ibk < BUNCH_ITER * (negative + 1); ibk++) {
          int ib = ibk / (negative + 1);
          int k = ibk - ib * (negative + 1);

          int target_on_text =
              (sentence_position + ib < sentence_length && k == 0);
          int neg_on_text =
              (sentence_position + ib < sentence_length && k != 0);

          int target = sen[sentence_position + ib];
          if (target_on_text) {
            bn_outputs[ibk] = target;
          }

          if (neg_on_text) {
            ulonglong tmp_random =
                (next_random + ibk + 1) * (ulonglong)(25214903917 + ibk) +
                11; // VECTORIZED RANDOM
            int negsample = table[(tmp_random >> 16) % table_size];

            int random_again = (!negsample || negsample == target);
            if (random_again) {
              tmp_random = tmp_random * (ulonglong)25214903917 + 11;
              negsample = table[(tmp_random >> 16) % table_size];
              random_again = (!negsample || negsample == target);
              if (random_again) {
                tmp_random = tmp_random * (ulonglong)25214903917 + 11;
                negsample = table[(tmp_random >> 16) % table_size];
              }
            }

            bn_outputs[ibk] = negsample;
          }
        }
#else  // W2V_USE_BUNCH_ITER
        // generate negative samples for output layer
        int offset = 1;
        for (int k = 0; k < negative; k++) {
          next_random = next_random * (ulonglong)25214903917 + 11;
          int sample = table[(next_random >> 16) % table_size];

          int random_again = (!sample || sample == target);
          if (random_again) {
            next_random = next_random * (ulonglong)25214903917 + 11;
            sample = table[(next_random >> 16) % table_size];
            random_again = (!sample || sample == target);
            if (random_again) {
              next_random = next_random * (ulonglong)25214903917 + 11;
              sample = table[(next_random >> 16) % table_size];
            }
          }

          outputs.indices[offset] = sample;
          outputs.meta[offset] = 1;
          offset++;
        }
        outputs.meta[0] = 1;
        outputs.length = offset;
#endif // W2V_USE_BUNCH_ITER

#ifdef W2V_USE_BUNCH_ITER
        for (int ib = 0; ib < BUNCH_ITER; ib++) {
          int input_size = bn_num_inputs[ib];
          int output_size = 1 + negative;
          int *pInOffsets = &bn_inputs[ib * batch_size];
          int *pOutOffsets = &bn_outputs[ib * output_size];
#else  // W2V_USE_BUNCH_ITER
          int input_size = num_inputs;
          int output_size = outputs.length;
          int *pInOffsets = inputs;
          int *pOutOffsets = outputs.indices;
#endif // W2V_USE_BUNCH_ITER

          if (input_size == 0 || pOutOffsets[0] == 0) {
            sentence_position++;
            if (sentence_position >= sentence_length) {
              sentence_length = 0;
#ifdef W2V_USE_BUNCH_ITER
              break;
#endif
            }
            continue;
          }

#if defined(USE_COMBINED_GEMM_ALL) || defined(USE_GEMM_FALLBACK)
#if defined(USE_COMBINED_GEMM_ALL)
          // w2v kernel(gemm0,1,2)
          if (are_supposed_parameters) {
              tableW2VKernel[input_size](output_size, hidden_size, input_size,
                                         alpha, pInOffsets, pOutOffsets, corrM, Woh,
                                         Wih);
          } else {
#endif   
            // fallback
            for (int i = 0; i < input_size; i++) {
              for (size_t j = 0; j < hidden_size; j++) {
                inputM[i * hidden_size + j] = Wih[pInOffsets[i] * hidden_size + j];
              }
            }
            
            for (int i = 0; i < output_size; i++) {
              for (size_t j = 0; j < hidden_size; j++) {
                outputM[i * hidden_size + j] = Woh[pOutOffsets[i] * hidden_size + j];
              }
            }
            w2v_gemm0(output_size, hidden_size, input_size, outputM, inputM, corrM);
            
            w2v_gemm0_associated(output_size, input_size, alpha, corrM);
            
            w2v_gemm1(output_size, hidden_size, input_size, corrM, inputM, outputMd);
            for (int i = 0; i < output_size; i++) {
              int src = i * hidden_size;
              int des = pOutOffsets[i] * hidden_size;
              for (int j = 0; j < hidden_size; j++) {
                Woh[des + j] += outputMd[src + j];
              }
            }
            
            w2v_gemm2(output_size, hidden_size, input_size, corrM, outputM, inputM);
            for (int i = 0; i < input_size; i++) {
              int src = i * hidden_size;
              int des = pInOffsets[i] * hidden_size;
              for (int j = 0; j < hidden_size; j++) {
                Wih[des + j] += inputM[src + j];
              }
            }
#if defined(USE_COMBINED_GEMM_ALL) 
          }
#endif
#elif defined(USE_COMBINED_GEMM_12) || defined(USE_COMBINED_GEMM_EACH)
// gemm0
#if defined(USE_VGEMM0_HU256)
          w2v_gemm0_HU256(output_size, hidden_size, input_size, pOutOffsets,
                      pInOffsets, Woh, Wih, outputMd, corrM);
#elif defined(USE_VGEMM0_HU512)
          w2v_gemm0_HU512_2X(output_size, hidden_size, input_size, pOutOffsets,
                         pInOffsets, Woh, Wih, outputMd, corrM);
#elif defined(USE_VGEMM0_HU512_WX)
          tableVGemm0[input_size](output_size, hidden_size, input_size, pOutOffsets,
                              pInOffsets, Woh, Wih, outputMd, corrM);
#else
          w2v_gemm0(output_size, hidden_size, input_size, pOutOffsets, pInOffsets,
                Woh, Wih, outputMd, corrM);
#endif

      // gemm0 associated
          w2v_gemm0_associated(output_size, input_size, alpha, corrM);

#ifdef USE_COMBINED_GEMM_12
          tableVGemm12[input_size](output_size, hidden_size, input_size, pInOffsets,
                               pOutOffsets, corrM, Woh, Wih);
#else // !USE_COMBINED_GEMM_12
// gemm1
#if defined(USE_VGEMM1_HU256)
          w2v_gemm1_HU256(output_size, hidden_size, input_size, pInOffsets,
                      pOutOffsets, corrM, Wih, Woh);
#elif defined(USE_VGEMM1_HU512)
          w2v_gemm1_HU512_2X(output_size, hidden_size, input_size, pInOffsets,
                         pOutOffsets, corrM, Wih, Woh);
#elif defined(USE_VGEMM1_N6_HU256)
          w2v_gemm1_N6_HU256(output_size, hidden_size, input_size, pInOffsets,
                         pOutOffsets, corrM, Wih, Woh);
#elif defined(USE_VGEMM1_N6_HU512)
          w2v_gemm1_N6_HU512_2X(output_size, hidden_size, input_size, pInOffsets,
                            pOutOffsets, corrM, Wih, Woh);
#else
          w2v_gemm1(output_size, hidden_size, input_size, pInOffsets, pOutOffsets,
                corrM, Wih, Woh);
#endif

// gemm2
#if defined(USE_VGEMM2_HU256)
          w2v_gemm2_HU256(output_size, hidden_size, input_size, pInOffsets, corrM,
                      outputMd, Wih);
#elif defined(USE_VGEMM2_HU512)
          w2v_gemm2_HU512_2X(output_size, hidden_size, input_size, pInOffsets,
                         corrM, outputMd, Wih);
#elif defined(USE_VGEMM2_HU256_WX)
          tableVGemm2HU256[input_size](output_size, hidden_size, input_size,
                                   pInOffsets, corrM, outputMd, Wih);
#elif defined(USE_VGEMM2_HU512_WX)
          tableVGemm2HU512[input_size](output_size, hidden_size, input_size,
                                   pInOffsets, corrM, outputMd, Wih);
#else
          w2v_gemm2(output_size, hidden_size, input_size, pInOffsets, corrM,
                outputMd, Wih);
#endif
#endif // USE_COMBINED_GEMM_12
#else  // !USE_COMBINED_GEMM_X
      // gemm0
          for (int i = 0; i < input_size; i++) {
            for (size_t j = 0; j < hidden_size; j++) {
              inputM[i * hidden_size + j] = Wih[pInOffsets[i] * hidden_size + j];
            }
          }

          for (int i = 0; i < output_size; i++) {
            for (size_t j = 0; j < hidden_size; j++) {
              outputM[i * hidden_size + j] = Woh[pOutOffsets[i] * hidden_size + j];
            }
          }
#if defined(USE_VGEMM0_HU256)
          w2v_gemm0_HU256(output_size, hidden_size, input_size, outputM, inputM,
                      corrM);
#elif defined(USE_VGEMM0_HU512)
          w2v_gemm0_HU512_2X(output_size, hidden_size, input_size, outputM, inputM,
                         corrM);
#elif defined(USE_VGEMM0_HU512_WX)
          tableVGemm0[input_size](output_size, hidden_size, input_size, outputM,
                              inputM, corrM);
#else
          w2v_gemm0(output_size, hidden_size, input_size, outputM, inputM, corrM);
#endif

          // gemm0 associated
          w2v_gemm0_associated(output_size, input_size, alpha, corrM);

// gemm1
#if defined(USE_VGEMM1_HU256)
          w2v_gemm1_HU256(output_size, hidden_size, input_size, corrM, inputM,
                      outputMd);
#elif defined(USE_VGEMM1_HU512)
          w2v_gemm1_HU512_2X(output_size, hidden_size, input_size, corrM, inputM,
                         outputMd);
#elif defined(USE_VGEMM1_N6_HU256)
          w2v_gemm1_N6_HU256(output_size, hidden_size, input_size, corrM, inputM,
                         outputMd);
#elif defined(USE_VGEMM1_N6_HU512)
          w2v_gemm1_N6_HU512_2X(output_size, hidden_size, input_size, corrM, inputM,
                            outputMd);
#else
          w2v_gemm1(output_size, hidden_size, input_size, corrM, inputM, outputMd);
#endif
          for (int i = 0; i < output_size; i++) {
            int src = i * hidden_size;
            int des = pOutOffsets[i] * hidden_size;
            for (int j = 0; j < hidden_size; j++) {
              Woh[des + j] += outputMd[src + j];
            }
          }

// gemm2
#if defined(USE_VGEMM2_HU256)
          w2v_gemm2_HU256(output_size, hidden_size, input_size, corrM, outputM,
                      inputM);
#elif defined(USE_VGEMM2_HU512)
          w2v_gemm2_HU512_2X(output_size, hidden_size, input_size, corrM, outputM,
                         inputM);
#elif defined(USE_VGEMM2_HU256_WX)
          tableVGemm2HU256[input_size](output_size, hidden_size, input_size, corrM,
                                   outputM, inputM);
#elif defined(USE_VGEMM2_HU512_WX)
          tableVGemm2HU512[input_size](output_size, hidden_size, input_size, corrM,
                                   outputM, inputM);
#else
          w2v_gemm2(output_size, hidden_size, input_size, corrM, outputM, inputM);
#endif
          for (int i = 0; i < input_size; i++) {
            int src = i * hidden_size;
            int des = pInOffsets[i] * hidden_size;
            for (int j = 0; j < hidden_size; j++) {
              Wih[des + j] += inputM[src + j];
            }
          }
#endif // USE_COMBINED_GEMM_X

          sentence_position++;
          if (sentence_position >= sentence_length) {
            sentence_length = 0;
#ifdef W2V_USE_BUNCH_ITER
            break;
#endif
          }
#ifdef W2V_USE_BUNCH_ITER
        } // for(ib)
#endif

      } // while(1)
#if !defined(USE_COMBINED_GEMM_ALL) && !defined(USE_COMBINED_GEMM_12) &&       \
    !defined(USE_COMBINED_GEMM_EACH)
      free(inputM);
      free(outputM);
#endif
#if !defined(USE_COMBINED_GEMM_ALL) && !defined(USE_COMBINED_GEMM_12)
      free(outputMd);
#endif
#if defined(USE_COMBINED_GEMM_ALL)
      if (!are_supposed_parameters) {
        free(inputM);
        free(outputM);
        free(outputMd);
      }
#endif
      free(corrM);
      free(pSenBase);
#ifdef W2V_USE_BUNCH_ITER
      free(bn_inputs);
      free(bn_outputs);
      free(bn_num_inputs);
      free(bn_b);
#endif

#ifdef W2V_USE_MPI
    } // if(id)
#endif


  } // omp parallel num_threads

#ifdef W2V_USE_MPI
  MPI_Barrier(frovedis::frovedis_comm_rpc);
#endif

}

}  // namespace


std::vector<float> train_each(std::vector<int> &_proc_train_data,
                              std::vector<int> &_vocab_count,
                              train_config &_config) {
  hidden_size = _config.hidden_size;
  window = _config.window;
  sample = _config.sample;
  negative = _config.negative;
  iter = _config.iter;
  //min_count = _config.min_count;
  alpha = _config.alpha;
  model_sync_period = _config.model_sync_period;
  min_sync_words = _config.min_sync_words;
  full_sync_times = _config.full_sync_times;
  message_size = _config.message_size;
  num_threads = _config.num_threads;
  
  batch_size = _config.window * 2 + 1;  // dependent

#ifdef W2V_USE_MPI
  MPI_Comm_size(frovedis::frovedis_comm_rpc, &num_procs);
  MPI_Comm_rank(frovedis::frovedis_comm_rpc, &my_rank);
#else
  num_procs = 1;
  my_rank = 0;
#endif

  proc_train_data = _proc_train_data.data();
  proc_train_words = _proc_train_data.size();
#ifdef W2V_USE_MPI
  MPI_Allreduce(&proc_train_words, &train_words, 1, MPI_LONG_LONG, MPI_SUM,
                frovedis::frovedis_comm_rpc);
#else
  train_words = proc_train_words;
#endif

  expTable = (real *)malloc((EXP_TABLE_SIZE + 1) * sizeof(real));
  for (int i = 0; i < EXP_TABLE_SIZE + 1; i++) {
    expTable[i] = exp((i / (real)EXP_TABLE_SIZE * 2 - 1) *
                      MAX_EXP); // Precompute the exp() table
    expTable[i] =
        expTable[i] / (expTable[i] + 1); // Precompute f(x) = x / (x + 1)
  }
  vocab_count = _vocab_count.data();
  vocab_size = _vocab_count.size();

  if (num_threads == 0) {
      frovedis::init_shm_info_local();
      int num_procs = omp_get_num_procs();
      int shm_size = frovedis::get_shm_size();
      if(shm_size == 0) {
        std::cerr << "WARN: frovedis::get_shm_size() returned 0, "
                  << "num_threads for OpenMP is set to 1" << std::endl;
        num_threads = 1;  
      }
      num_threads = std::max(num_procs / shm_size, 1);
  }

  Train_SGNS_general();

  std::vector<float> ret;
  if (my_rank == 0) {
    ret.assign(Wih, Wih + vocab_size * hidden_size);
  }

  free(Wih);
  free(Woh);
  free(expTable);
  free(table);

  return ret;
}

std::vector<float> 
train_each_impl(std::string &_train_file_str,
                std::string &_vocab_count_file_str,
                train_config &_config) {
#ifdef W2V_USE_MPI
  MPI_Comm_size(frovedis::frovedis_comm_rpc, &num_procs);
  MPI_Comm_rank(frovedis::frovedis_comm_rpc, &my_rank);
#else
  num_procs = 1;
  my_rank = 0;
#endif

  std::vector<int> _train_data;
  if (my_rank == 0) {
    _train_data = read_vector_from_binary_file<int>(_train_file_str);
    train_words = _train_data.size();
  }

#ifdef W2V_USE_MPI
  MPI_Bcast(&train_words, 1, MPI_LONG_LONG, 0, frovedis::frovedis_comm_rpc);

  ulonglong step =
      train_words / num_procs + (train_words / num_procs > 0 ? 1 : 0);
  std::vector<int> counts(num_procs);
  std::vector<int> displs(num_procs);
  for (size_t i = 0; i < num_procs; i++) {
    displs[i] = (i * step <= train_words) ? i * step : train_words;
    counts[i] = ((i + 1) * step <= train_words)
                    ? step
                    : std::max(train_words - displs[i], (ulonglong)0);
  }

  std::vector<int> _proc_train_data(counts[my_rank]);

  MPI_Scatterv(_train_data.data(), counts.data(), displs.data(), MPI_INT,
               _proc_train_data.data(), counts[my_rank], MPI_INT, 0,
               frovedis::frovedis_comm_rpc);
  proc_train_words = counts[my_rank];
  // release
  std::vector<int>().swap(_train_data);
#else
  proc_train_words = _train_data.size();
  std::vector<int> &_proc_train_data = _train_data;
#endif

  std::vector<int> _vocab_count =
      read_vector_from_binary_file<int>(_vocab_count_file_str);

  return train_each(_proc_train_data, _vocab_count, _config);
}

void train_each(std::string &_train_file_str,
                std::string &_vocab_count_file_str,
                std::string &_weight_file_str,
                train_config &_config) {
  auto weight = train_each_impl(_train_file_str, _vocab_count_file_str, 
                                _config);
#ifdef W2V_USE_MPI
  MPI_Comm_rank(frovedis::frovedis_comm_rpc, &my_rank);
#else
  my_rank = 0;
#endif
  if (my_rank == 0) write_vector_to_binary_file(weight, _weight_file_str);
}

} // namespace w2v
