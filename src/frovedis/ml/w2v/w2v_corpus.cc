
#include "w2v_corpus.hpp"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

#include "w2v_common.hpp"

#define MAX_STRING 100


namespace w2v {

namespace {
  
struct vocab_word *vocab = NULL;
int *vocab_hash = NULL;
// int *vocab_count = NULL;
int min_count = 5, min_reduce = 1;
int vocab_max_size = 1000, vocab_size = 0, hidden_size = 100;
ulonglong train_words = 0, file_size = 0;
char train_file[MAX_STRING], output_file[MAX_STRING];
char save_vocab_file[MAX_STRING], read_vocab_file[MAX_STRING];

const int vocab_hash_size = 30000000;

real *Wih = NULL;


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

int GetWordHash(char *word) {
  uint hash = 0;
  for (int i = 0; i < strlen(word); i++)
  hash = hash * 257 + word[i];
  hash = hash % vocab_hash_size;
  return hash;
}

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

int ReadWordIndex(FILE *fin) {
  char word[MAX_STRING];
  ReadWord(word, fin);
  if (feof(fin))
  return -1;
  return SearchVocab(word);
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

int VocabCompare(const void *a, const void *b) {
    return ((struct vocab_word *) b)->cn - ((struct vocab_word *) a)->cn;
}

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
        // if (my_rank == 0 && debug_mode > 1 && (train_words % 100000 == 0) && get_loglevel() <= DEBUG) {
        //     printf("%lldK%c", train_words / 1000, 13);
        //     fflush(stdout);
        // }
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
    // if (my_rank == 0 && debug_mode > 0 && get_loglevel() <= DEBUG) {
    //     printf("Vocab size: %d\n", vocab_size);
    //     printf("Words in train file: %lld\n", train_words);
    // }
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
    // if (debug_mode > 0 && my_rank == 0 && get_loglevel() <= DEBUG) {
    //     printf("Vocab size: %d\n", vocab_size);
    //     printf("Words in train file: %lld\n", train_words);
    // }
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

void saveModel(bool binary) {
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

}  // namespace


void build_vocab_and_dump(const std::string& _train_file_str, const std::string& _stream_file_str,
                          const std::string& _vocab_file_str, const std::string& _vocab_count_file_str,
                          int _min_count) {

  vocab = (struct vocab_word *) calloc(vocab_max_size, sizeof(struct vocab_word));
  vocab_hash = (int *) malloc(vocab_hash_size * sizeof(int));
  min_count = _min_count;

  strcpy(train_file, _train_file_str.c_str());
  LearnVocabFromTrainFile();

  strcpy(save_vocab_file, _vocab_file_str.c_str());
  SaveVocab();

  std::vector<int> vocab_count_vec(vocab_size, 0);
  for (size_t i = 0; i < vocab_size; i++) {
    vocab_count_vec[i] = vocab[i].cn;
  }
  
  // make_dvector_scatter(vocab_count_vec).savebinary(_vocab_count_file_str);
  write_vector_to_binary_file<int>(vocab_count_vec, _vocab_count_file_str);

  FILE *fin = fopen(train_file, "rb");
  std::vector<int> stream_vec(train_words + 1, 0);
  loadStream(fin, stream_vec.data(), train_words);
  fclose(fin);
  // make_dvector_scatter(stream_vec).savebinary(_stream_file_str);
  write_vector_to_binary_file<int>(stream_vec, _stream_file_str);

  for (int i = 0; i < vocab_size; i++)  free(vocab[i].word);
  free(vocab);
  free(vocab_hash);
}

void save_model(const std::vector<real>& _weight,
                const std::string& _vocab_file_str,
                const std::string& _output_file_str,
                const int _min_count,
                bool _binary)
{
  min_count = _min_count;

  vocab = (struct vocab_word *) calloc(vocab_max_size, sizeof(struct vocab_word));
  vocab_hash = (int *) malloc(vocab_hash_size * sizeof(int));

  strcpy(read_vocab_file, _vocab_file_str.c_str());
  ReadVocab();

  strcpy(output_file, _output_file_str.c_str());
  
  //auto weight = read_vector_from_binary_file<float>(_weight_file_str);
  hidden_size = _weight.size() / vocab_size;

  /*
  Wih = (real *) malloc(_weight.size() * sizeof(real));
  for (size_t i = 0; i < _weight.size(); i++) {
    Wih[i] = _weight[i];
  }
  */
  const real* _weight_ptr = _weight.data();
  Wih = const_cast<real*>(_weight_ptr);

  saveModel(_binary);
  
  //free(Wih);
  for (int i = 0; i < vocab_size; i++)  free(vocab[i].word);
  free(vocab);
  free(vocab_hash);
}
  
void save_model(const std::string& _weight_file_str,
                const std::string& _vocab_file_str,
                const std::string& _output_file_str,
                const int _min_count,
                bool _binary)
{
  auto _weight = read_vector_from_binary_file<float>(_weight_file_str);
  save_model(_weight, _vocab_file_str, _output_file_str, _min_count, _binary);
}


}  // namespace w2v
