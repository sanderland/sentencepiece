#include <iostream>
#include <string>
#include <vector>

#include "src/unigram_model_trainer.h"
#include "src/trainer_interface.h"

// Expose protected LoadSentences and public MakeSeedSentencePieces
class ExposedTrainer : public sentencepiece::unigram::Trainer {
 public:
  using sentencepiece::unigram::Trainer::Trainer;
  using sentencepiece::TrainerInterface::LoadSentences;
  using sentencepiece::unigram::Trainer::MakeSeedSentencePieces;
};

int main() {
  using namespace sentencepiece;
  using namespace sentencepiece::unigram;

  // Prepare tiny corpus file
  const std::string corpus_path = "seed_corpus.txt";
  {
    FILE* f = fopen(corpus_path.c_str(), "w");
    if (!f) { perror("fopen"); return 1; }
    fprintf(f, "November\nNovember\nDecember\nDecember\n");
    fclose(f);
  }

  // Build specs
  TrainerSpec spec;
  spec.set_model_type(TrainerSpec::UNIGRAM);
  spec.set_vocab_size(200);
  spec.set_seed_sentencepiece_size(1000);
  spec.set_character_coverage(1.0);
  spec.add_input(corpus_path);
  spec.set_num_threads(1);
  // Keep defaults for meta pieces

  NormalizerSpec normalizer_spec;  // defaults are fine
  normalizer_spec.set_escape_whitespaces(true);
  NormalizerSpec denormalizer_spec;

  ExposedTrainer trainer(spec, normalizer_spec, denormalizer_spec);
  auto st = trainer.status();
  if (!st.ok()) {
    std::cerr << "Trainer status not ok: " << st.ToString() << std::endl;
    return 1;
  }

  auto ls = trainer.LoadSentences();
  if (!ls.ok()) {
    std::cerr << "LoadSentences failed: " << ls.ToString() << std::endl;
    return 1;
  }

  auto seeds = trainer.MakeSeedSentencePieces();
  std::cout << "Seed pieces (size=" << seeds.size() << ")\n";
  size_t shown = 0;
  for (const auto& kv : seeds) {
    if (shown < 50) {
      std::cout << "  '" << kv.first << "' (score/logprob: " << kv.second << ")\n";
    }
    shown++;
  }
  bool has_nov = false, has_dec = false;
  for (const auto& kv : seeds) {
    if (kv.first == "November") has_nov = true;
    if (kv.first == "December") has_dec = true;
  }
  std::cout << "Contains 'November'? " << (has_nov ? "yes" : "no") << "\n";
  std::cout << "Contains 'December'? " << (has_dec ? "yes" : "no") << "\n";

  return 0;
}


