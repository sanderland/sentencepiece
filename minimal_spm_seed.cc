#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "third_party/esaxx/esa.hxx"

int main() {
  // Small corpus with repeated "November" to demonstrate delimiter behavior
  std::vector<std::string> texts = {
      "November",
      "November",
      "December",
      "December",
  };

  std::cout << "Input texts:\n";
  for (const auto& t : texts) {
    std::cout << "- \"" << t << "\"\n";
  }

  // Build corpus with NUL separators (byte-level)
  std::string corpus;
  for (size_t i = 0; i < texts.size(); ++i) {
    if (!corpus.empty()) corpus.push_back('\0'); // separator between sentences
    corpus += texts[i];
    corpus.push_back('\0'); // boundary after each sentence (like SentencePiece)
  }

  const int n = static_cast<int>(corpus.size());
  std::vector<int> s(n);
  for (int i = 0; i < n; ++i) s[i] = static_cast<unsigned char>(corpus[i]);

  std::vector<int> SA(n), L(n), R(n), D(n);
  int node_num = 0;
  constexpr int kAlphabet = 256;
  if (esaxx(s.begin(), SA.begin(), L.begin(), R.begin(), D.begin(), n, kAlphabet,
            node_num) != 0) {
    std::cerr << "esaxx failed" << std::endl;
    return 1;
  }

  // Extract substrings from internal nodes, two modes:
  // 1) strict_skip_delim: skip candidates containing the delimiter (SentencePiece-like)
  // 2) trimmed_before_delim: emit the prefix up to (but not including) the first delimiter
  std::unordered_map<std::string, int> strict_skip_delim;
  std::unordered_map<std::string, int> trimmed_before_delim;
  for (int i = node_num - 1; i >= 0; --i) {
    const int offset = SA[L[i]];
    const int len = D[i];
    if (len <= 1 || offset < 0 || offset + len > n) continue;
    const std::string sub = corpus.substr(offset, len);
    const int freq = R[i] - L[i];
    if (freq <= 1) continue;

    // Mode 1: SentencePiece-like strict behavior (skip spans with delimiter)
    if (sub.find('\0') == std::string::npos && static_cast<int>(sub.size()) <= 100) {
      strict_skip_delim[sub] = freq;
    }

    // Mode 2: Trim at first delimiter, so we keep the portion before it
    {
      size_t pos = sub.find('\0');
      const std::string trimmed = (pos == std::string::npos) ? sub : sub.substr(0, pos);
      if (trimmed.size() > 1 && static_cast<int>(trimmed.size()) <= 100) {
        trimmed_before_delim[trimmed] += freq;
      }
    }
  }

  // Note: single-character additions removed for educational parity with Python when disabled.

  auto print_map = [](const char* title,
                      const std::unordered_map<std::string, int>& m) {
    std::vector<std::pair<std::string, int>> items(m.begin(), m.end());
    std::sort(items.begin(), items.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });
    std::cout << "\n=== " << title << " ===\n";
    std::cout << "Found " << items.size() << " unique substrings:\n";
    for (size_t i = 0; i < items.size(); ++i) {
      std::cout << "  " << std::setw(3) << static_cast<int>(i + 1) << ". '"
                << items[i].first << "' (freq: " << items[i].second << ")\n";
    }
  };

  print_map("Strict (skip delimiter)", strict_skip_delim);
  print_map("Trimmed before delimiter", trimmed_before_delim);

  auto find_word = [](const std::unordered_map<std::string, int>& m, const std::string& w) {
    auto it = m.find(w);
    if (it == m.end()) return std::make_pair(false, 0);
    return std::make_pair(true, it->second);
  };

  const std::string probe = "November";
  const auto s_strict = find_word(strict_skip_delim, probe);
  const auto s_trim = find_word(trimmed_before_delim, probe);
  std::cout << "\nProbe '" << probe << "':\n";
  std::cout << "  Strict:  present=" << std::boolalpha << s_strict.first
            << ", freq=" << s_strict.second << "\n";
  std::cout << "  Trimmed: present=" << std::boolalpha << s_trim.first
            << ", freq=" << s_trim.second << "\n";

  int single_char = 0, multi_char = 0, total_len = 0, total_freq = 0;
  std::string longest;
  size_t maxlen = 0;
  for (const auto &kv : trimmed_before_delim) {
    if (kv.first.size() == 1) ++single_char; else ++multi_char;
    total_len += static_cast<int>(kv.first.size());
    total_freq += kv.second;
    if (kv.first.size() > maxlen) {
      maxlen = kv.first.size();
      longest = kv.first;
    }
  }

  std::cout << "\nStatistics:\n";
  std::cout << "  - Total unique substrings: " << trimmed_before_delim.size() << "\n";
  std::cout << "  - Single character substrings: " << single_char << "\n";
  std::cout << "  - Multi character substrings: " << multi_char << "\n";
  std::cout << std::fixed << std::setprecision(2);
  if (!trimmed_before_delim.empty())
    std::cout << "  - Average substring length (trimmed set): "
              << static_cast<double>(total_len) / trimmed_before_delim.size() << "\n";
  std::cout << "  - Longest substring: '" << longest << "' (" << maxlen
            << " chars)\n";
  std::cout << "  - Total frequency count: " << total_freq << "\n";

  return 0;
}
