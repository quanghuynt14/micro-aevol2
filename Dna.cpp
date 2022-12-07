//
// Created by arrouan on 01/10/18.
//

#include "Dna.h"

#include <cassert>

Dna::Dna(std::vector<char> & big_dna, int start_pos, int length, Threefry::Gen &&rng) : big_dna_(big_dna), start_pos_(start_pos), length_(length) {
    // Generate a random genome
    for (int32_t i = start_pos; i < start_pos + length; i++) {
        big_dna_[i] = '0' + rng.random(NB_BASE);
    }
}

Dna::Dna(std::vector<char> & big_dna) : big_dna_(big_dna) {

}

int Dna::length() const {
    return length_;
}

std::vector<char> Dna::get_seq() {
    std::vector<char>::const_iterator first = big_dna_.begin() + start_pos_;
    std::vector<char>::const_iterator last = big_dna_.begin() + start_pos_ + length_;
    std::vector<char> seq(first, last);
    return seq;
}

void Dna::save(gzFile backup_file) {
    int dna_length = length();
    gzwrite(backup_file, &dna_length, sizeof(dna_length));

    std::vector<char> seq = get_seq();
    gzwrite(backup_file, seq.data(), dna_length * sizeof(seq[0]));
}

void Dna::load(gzFile backup_file) {
    int dna_length;
    gzread(backup_file, &dna_length, sizeof(dna_length));

    char tmp_seq[dna_length];
    gzread(backup_file, tmp_seq, dna_length * sizeof(tmp_seq[0]));

    std::vector<char> seq = std::vector<char>(tmp_seq, tmp_seq + dna_length);

    for (int32_t i = 0; i < length_; i++) {
        big_dna_[start_pos_ + i] = seq[i];
    }
}

void Dna::set(int pos, char c) {
    big_dna_[start_pos_ + pos] = c;
}

/**
 * Remove the DNA inbetween pos_1 and pos_2
 *
 * @param pos_1
 * @param pos_2
 */
void Dna::remove(int pos_1, int pos_2) {
    assert(pos_1 >= 0 && pos_2 >= pos_1 && pos_2 <= length_);

    std::vector<char> seq = get_seq();
    seq.erase(seq.begin() + pos_1, seq.begin() + pos_2);

    length_ = seq.size();

    for (int32_t i = 0; i < length_; i++) {
        big_dna_[start_pos_ + i] = seq[i];
    }

}

/**
 * Insert a sequence of a given length at a given position into the DNA of the Organism
 *
 * @param pos : where to insert the sequence
 * @param seq : the sequence itself
 * @param seq_length : the size of the sequence
 */
void Dna::insert(int pos, std::vector<char> seq_insert) {
// Insert sequence 'seq' at position 'pos'
    assert(pos >= 0 && pos < length_);

    std::vector<char> seq = get_seq();
    seq.insert(seq.begin() + pos, seq_insert.begin(), seq_insert.end());

    length_ = seq.size();

    for (int32_t i = 0; i < length_; i++) {
        big_dna_[start_pos_ + i] = seq[i];
    }
}

/**
 * Insert a sequence of a given length at a given position into the DNA of the Organism
 *
 * @param pos : where to insert the sequence
 * @param seq : the sequence itself
 * @param seq_length : the size of the sequence
 */
void Dna::insert(int pos, Dna *seq_insert) {
// Insert sequence 'seq' at position 'pos'
    assert(pos >= 0 && pos < length_);

    std::vector<char> seq = get_seq();
    seq.insert(seq.begin() + pos, seq_insert->get_seq().begin(), seq_insert->get_seq().end());

    length_ = seq.size();

    for (int32_t i = 0; i < length_; i++) {
        big_dna_[start_pos_ + i] = seq[i];
    }
}

void Dna::do_switch(int pos) {
    if (big_dna_[start_pos_ + pos] == '0') big_dna_[start_pos_ + pos] = '1';
    else big_dna_[start_pos_ + pos] = '0';
}

void Dna::do_duplication(int pos_1, int pos_2, int pos_3) {
    // Duplicate segment [pos_1; pos_2[ and insert the duplicate before pos_3
    char *duplicate_segment = NULL;

    int32_t seg_length;

    std::vector<char> seq = get_seq();

    if (pos_1 < pos_2) {
        //
        //       pos_1         pos_2                   -> 0-
        //         |             |                   -       -
        // 0--------------------------------->      -         -
        //         ===============                  -         - pos_1
        //           tmp (copy)                      -       -
        //                                             -----      |
        //                                             pos_2    <-'
        //
        std::vector<char> seq_dupl =
                std::vector<char>(seq.begin() + pos_1, seq.begin() + pos_2);

        insert(pos_3, seq_dupl);
    } else { // if (pos_1 >= pos_2)
        // The segment to duplicate includes the origin of replication.
        // The copying process will be done in two steps.
        //
        //                                            ,->
        //    pos_2                 pos_1            |      -> 0-
        //      |                     |                   -       - pos_2
        // 0--------------------------------->     pos_1 -         -
        // ======                     =======            -         -
        //  tmp2                        tmp1              -       -
        //                                                  -----
        //
        //
        std::vector<char>
                seq_dupl = std::vector<char>(seq.begin() + pos_1, seq.end());
        seq_dupl.insert(seq_dupl.end(), seq.begin(), seq.begin() + pos_2);

        insert(pos_3, seq_dupl);
    }
}

int Dna::promoter_at(int pos) {
    // int prom_dist[PROM_SIZE];
    int dist_lead = 0;

    for (int motif_id = 0; motif_id < PROM_SIZE; motif_id++) {
        int search_pos = pos + motif_id;
        if (search_pos >= length_)
            search_pos -= length_;
        // Searching for the promoter
        // prom_dist[motif_id] =
        dist_lead +=
                PROM_SEQ[motif_id] == big_dna_[start_pos_ + search_pos] ? 0 : 1;

    }


    // Computing if a promoter exists at that position
    // int dist_lead = prom_dist[0] +
    //                 prom_dist[1] +
    //                 prom_dist[2] +
    //                 prom_dist[3] +
    //                 prom_dist[4] +
    //                 prom_dist[5] +
    //                 prom_dist[6] +
    //                 prom_dist[7] +
    //                 prom_dist[8] +
    //                 prom_dist[9] +
    //                 prom_dist[10] +
    //                 prom_dist[11] +
    //                 prom_dist[12] +
    //                 prom_dist[13] +
    //                 prom_dist[14] +
    //                 prom_dist[15] +
    //                 prom_dist[16] +
    //                 prom_dist[17] +
    //                 prom_dist[18] +
    //                 prom_dist[19] +
    //                 prom_dist[20] +
    //                 prom_dist[21];

    return dist_lead;
}

// Given a, b, c, d boolean variable and X random boolean variable,
// a terminator look like : a b c d X X !d !c !b !a
int Dna::terminator_at(int pos) {
    // int term_dist[TERM_STEM_SIZE];
    int dist_term_lead = 0;

    for (int motif_id = 0; motif_id < TERM_STEM_SIZE; motif_id++) {
        int right = pos + motif_id;
        int left = pos + (TERM_SIZE - 1) - motif_id;

        // loop back the dna inf needed
        if (right >= length()) right -= length();
        if (left >= length()) left -= length();

        // Search for the terminators
        // term_dist[motif_id] 
        dist_term_lead += 
                big_dna_[start_pos_ + right] != big_dna_[start_pos_ + left] ? 1 : 0;
    }
    // int dist_term_lead = term_dist[0] +
    //                      term_dist[1] +
    //                      term_dist[2] +
    //                      term_dist[3];

    return dist_term_lead;
}

bool Dna::shine_dal_start(int pos) {
    bool start = false;
    int t_pos, k_t;

    for (int k = 0; k < SHINE_DAL_SIZE + CODON_SIZE; k++) {
        k_t = k >= SHINE_DAL_SIZE ? k + SD_START_SPACER : k;
        t_pos = pos + k_t;
        if (t_pos >= length_)
            t_pos -= length_;

        if (big_dna_[start_pos_ + t_pos] == SHINE_DAL_SEQ[k_t]) {
            start = true;
        } else {
            start = false;
            break;
        }
    }

    return start;
}

bool Dna::protein_stop(int pos) {
    bool is_protein;
    int t_k;

    for (int k = 0; k < CODON_SIZE; k++) {
        t_k = pos + k;
        if (t_k >= length_)
            t_k -= length_;

        if (big_dna_[start_pos_ + t_k] == PROTEIN_END[k]) {
            is_protein = true;
        } else {
            is_protein = false;
            break;
        }
    }

    return is_protein;
}

int Dna::codon_at(int pos) {
    int value = 0;

    int t_pos;

    for (int i = 0; i < CODON_SIZE; i++) {
        t_pos = pos + i;
        if (t_pos >= length_)
            t_pos -= length_;
        if (big_dna_[start_pos_ + t_pos] == '1')
            value += 1 << (CODON_SIZE - i - 1);
    }

    return value;
}