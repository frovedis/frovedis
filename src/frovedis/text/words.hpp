#ifndef WORDS_HPP
#define WORDS_HPP

#include <vector>
#include <string>
#include "../core/mpi_rpc.hpp"
#include "../core/serializehelper.hpp"

#if defined(__ve__) || defined(_SX)
#define WORDS_VLEN 256
#define WORDS_VECTOR_BLOCK 4096
#else
//#define WORDS_VLEN 1
#define WORDS_VLEN 4
#define WORDS_VECTOR_BLOCK 4
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

void substr(size_t* starts, size_t* lens, size_t num_words,
            size_t pos, size_t num);
               
void substr(std::vector<size_t>& starts,
            std::vector<size_t>& lens,
            size_t pos, size_t num);

void substr(size_t* starts, size_t* lens, size_t num_words,
            size_t pos);
               
void substr(std::vector<size_t>& starts,
            std::vector<size_t>& lens,
            size_t pos);

/*
  idx: index of words that contains the string
  pos: position inside of the word
  if there are multiple occurrence in one word, same index is used.
  e.g. words: {"abracadabra", "abra"}, to_search: "bra"
  => idx: {0,0,1}, pos: {1,8,1}
*/
void search(const std::vector<int>& chars,
            const std::vector<size_t>& starts,
            const std::vector<size_t>& lens,
            const std::string& to_search,
            std::vector<size_t>& idx, std::vector<size_t>& pos);
void replace(const std::vector<int>& chars,
             const std::vector<size_t>& starts,
             const std::vector<size_t>& lens,
             std::vector<int>& ret_chars,
             std::vector<size_t>& ret_starts,
             std::vector<size_t>& ret_lens,
             const std::string& from,
             const std::string& to);
void prepend(const std::vector<int>& chars,
             const std::vector<size_t>& starts,
             const std::vector<size_t>& lens,
             std::vector<int>& ret_chars,
             std::vector<size_t>& ret_starts,
             std::vector<size_t>& ret_lens,
             const std::string& to_prepend);
void append(const std::vector<int>& chars,
            const std::vector<size_t>& starts,
            const std::vector<size_t>& lens,
            std::vector<int>& ret_chars,
            std::vector<size_t>& ret_starts,
            std::vector<size_t>& ret_lens,
            const std::string& to_append);

// utility struct
struct words {
  std::vector<int> chars;
  std::vector<size_t> starts;
  std::vector<size_t> lens;

  void clear() { // to free memory
    std::vector<int> chars_tmp; chars.swap(chars_tmp);
    std::vector<size_t> starts_tmp; starts.swap(starts_tmp);
    std::vector<size_t> lens_tmp; lens.swap(lens_tmp);
  }
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
  void substr(size_t pos, size_t num)
    {frovedis::substr(starts, lens, pos, num);}
  void substr(size_t pos)
    {frovedis::substr(starts, lens, pos);}
  void replace(const std::string& from, const std::string& to) { // destructive
    std::vector<int> ret_chars;
    std::vector<size_t> ret_starts, ret_lens;
    frovedis::replace(chars, starts, lens, ret_chars, ret_starts, ret_lens,
                      from, to);
    chars.swap(ret_chars);
    starts.swap(ret_starts);
    lens.swap(ret_lens);
  }
  void prepend(const std::string& to_prepend) { // destructive
    std::vector<int> ret_chars;
    std::vector<size_t> ret_starts, ret_lens;
    frovedis::prepend(chars, starts, lens, ret_chars, ret_starts, ret_lens,
                      to_prepend);
    chars.swap(ret_chars);
    starts.swap(ret_starts);
    lens.swap(ret_lens);
  }
  void append(const std::string& to_append) { // destructive
    std::vector<int> ret_chars;
    std::vector<size_t> ret_starts, ret_lens;
    frovedis::append(chars, starts, lens, ret_chars, ret_starts, ret_lens,
                      to_append);
    chars.swap(ret_chars);
    starts.swap(ret_starts);
    lens.swap(ret_lens);
  }

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

words merge_multi_words(const std::vector<words>&);

std::vector<int> concat_words(const std::vector<int>& v,
                              const std::vector<size_t>& starts,
                              const std::vector<size_t>& len,
                              const std::string& delim,
                              std::vector<size_t>& new_starts);

std::vector<int> concat_words(const words& w, const std::string& delim,
                              std::vector<size_t>& new_starts);

std::vector<size_t> like(const std::vector<int>& chars,
                         const std::vector<size_t>& starts,
                         const std::vector<size_t>& lens,
                         const std::string& to_search,
                         int wild_card = '%', int escape = '\\');
std::vector<size_t> like(const words& w, const std::string& to_search,
                         int wild_card = '%', int escape = '\\');

// for saving CSV file
void quote_and_escape(words& ws);

words vector_string_to_words(const std::vector<std::string>& str);
std::vector<std::string> words_to_vector_string(const words& ws);

void search(const words& w, const std::string& to_search,
            std::vector<size_t>& idx, std::vector<size_t>& pos);
words replace(const words& w, const std::string& from, const std::string& to);
words prepend(const words& w, const std::string& to_prepend);
words append(const words& w, const std::string& to_append);
words horizontal_concat_words(std::vector<words>& vec_words);

}
#endif
