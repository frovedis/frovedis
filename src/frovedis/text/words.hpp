#ifndef WORDS_HPP
#define WORDS_HPP

#include "../core/mpi_rpc.hpp"
#include "../core/serializehelper.hpp"

#if defined(__ve__) || defined(_SX)
#define WORDS_VLEN 256
#else
//#define WORDS_VLEN 1
#define WORDS_VLEN 4
#endif

namespace frovedis {

void remove_doc_heading(const std::vector<int>& v,
                        std::vector<size_t>& sep,
                        std::vector<size_t>& len,
                        const std::string& del);

std::vector<int> concat_docs(const std::vector<int>& v,
                             const std::vector<size_t>& sep,
                             const std::vector<size_t>& len,
                             const int del,
                             std::vector<size_t>& doc_start);

// delims is set of characters used as delimiter
void split_to_words(const std::vector<int>& v,
                    std::vector<size_t>& starts,
                    std::vector<size_t>& lens,
                    const std::string& delims);

// "starts" start from first item of the original vector
// offset is used to add the difference between the original vector and vp
void split_to_words(const int* vp, size_t size, size_t offset,
                    std::vector<size_t>& starts,
                    std::vector<size_t>& lens,
                    const std::string& delims);

void print_words(const int* vp,
                 size_t* starts, size_t* lens, size_t num_words);
void print_words(const std::vector<int>& v,
                 std::vector<size_t>& starts,
                 std::vector<size_t>& lens);

void trim_head(const int* vp,
               size_t* starts, size_t* lens, size_t num_words,
               const std::string& to_trim);
               
void trim_head(const std::vector<int>& v,
               std::vector<size_t>& starts,
               std::vector<size_t>& lens,
               const std::string& to_trim);

void trim_tail(const int* vp,
               size_t* starts, size_t* lens, size_t num_words,
               const std::string& to_trim);
               
void trim_tail(const std::vector<int>& v,
               std::vector<size_t>& starts,
               std::vector<size_t>& lens,
               const std::string& to_trim);

void trim(const int* vp,
          size_t* starts, size_t* lens, size_t num_words,
          const std::string& to_trim);
               
void trim(const std::vector<int>& v,
          std::vector<size_t>& starts,
          std::vector<size_t>& lens,
          const std::string& to_trim);

void trim_noalpha_head(const int* vp,
                       size_t* starts, size_t* lens, size_t num_words);
               
void trim_noalpha_head(const std::vector<int>& v,
                       std::vector<size_t>& starts,
                       std::vector<size_t>& lens);

void trim_noalpha_tail(const int* vp,
                       size_t* starts, size_t* lens, size_t num_words);
               
void trim_noalpha_tail(const std::vector<int>& v,
                       std::vector<size_t>& starts,
                       std::vector<size_t>& lens);

void trim_noalpha(const int* vp,
                  size_t* starts, size_t* lens, size_t num_words);
               
void trim_noalpha(const std::vector<int>& v,
                  std::vector<size_t>& starts,
                  std::vector<size_t>& lens);

void remove_null(size_t* starts, size_t* lens, size_t& num_words);
               
void remove_null(std::vector<size_t>& starts,
                 std::vector<size_t>& lens);

// utility struct
struct words {
  std::vector<int> chars;
  std::vector<size_t> starts;
  std::vector<size_t> lens;

  void print()
    {print_words(chars, starts, lens);}
  void trim_head(const std::string& to_trim)
    {frovedis::trim_head(chars, starts, lens, to_trim);}
  void trim_tail(const std::string& to_trim)
    {frovedis::trim_tail(chars, starts, lens, to_trim);}
  void trim(const std::string& to_trim)
    {frovedis::trim(chars, starts, lens, to_trim);}
  void trim_noalpha_head()
    {frovedis::trim_noalpha_head(chars, starts, lens);}
  void trim_noalpha_tail()
    {frovedis::trim_noalpha_tail(chars, starts, lens);}
  void trim_noalpha()
    {frovedis::trim_noalpha(chars, starts, lens);}
  void remove_null()
    {frovedis::remove_null(starts, lens);}

  SERIALIZE(chars, starts, lens)
};

words split_to_words(const std::vector<int>& v, const std::string& delims);

// used for document starting point, position of sentence delimiter (period)
// to word count position (you can use find to get sentence delimiter)
// lower_bound is used, so points to the same or next word
std::vector<size_t>
convert_position_to_word_count(const std::vector<size_t>& doc_starts,
                               const std::vector<size_t>& word_starts);

words merge_words(const words& a, const words& b);

std::vector<int> concat_words(const std::vector<int>& v,
                              const std::vector<size_t>& starts,
                              const std::vector<size_t>& len,
                              const std::string& delim,
                              std::vector<size_t>& new_starts);

std::vector<int> concat_words(const words& w, const std::string& delim,
                              std::vector<size_t>& new_starts);

}
#endif
