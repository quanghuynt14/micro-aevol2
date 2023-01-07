//
// Created by arrouan on 01/10/18.
//

#include "Dna.h"

#include <cassert>
#include <string.h>

Dna::Dna(int length, Threefry::Gen &&rng) : length_(length) {
    seq_ = new char[length];
    // Generate a random genome
    for (int32_t i = 0; i < length; i++) {
        seq_[i] = '0' + rng.random(NB_BASE);
    }
}

Dna::Dna(const Dna &clone) {
    length_ = clone.length_;
    
    seq_ = new char[length_];
    memcpy(seq_, clone.seq_, length_ * sizeof(char));
    
}

void Dna::save(gzFile backup_file) {
    gzwrite(backup_file, &length_, sizeof(length_));
    gzwrite(backup_file, seq_, length_ * sizeof(seq_[0]));
}

void Dna::load(gzFile backup_file) {
    int dna_length;
    gzread(backup_file, &dna_length, sizeof(dna_length));
    length_ = dna_length;

    char tmp_seq[dna_length];
    gzread(backup_file, tmp_seq, dna_length * sizeof(tmp_seq[0]));
    seq_ = tmp_seq;
}

void Dna::set(int pos, char c) {
    seq_[pos] = c;
}

/**
 * Remove the DNA a range of elements [pos_1, pos_2)
 * including the element pointed by pos_1 but not the one pointed by pos_2.
 *
 * @param pos_1
 * @param pos_2
 */
void Dna::remove(int pos_1, int pos_2) {
    assert(pos_1 >= 0 && pos_2 > pos_1 && pos_2 <= length_);

    int new_length = length_ - (pos_2 - pos_1);
    char* new_seq = new char[new_length];

    for (auto i = 0; i < pos_1; i++) {
        new_seq[i] = seq_[i];
    }

    for (auto i = pos_1; i < new_length; i++) {
        new_seq[i] = seq_[i + (pos_2 - pos_1)];
    }
    
    length_ = new_length;
    seq_ = new_seq;
}

/**
 * Insert a sequence of a given length before a given position into the DNA of the Organism
 *
 * @param pos : where to insert the sequence
 * @param seq : the sequence itself
 * @param seq_length : the size of the sequence
 */
void Dna::insert(int pos, const char* seq, int seq_length) {
// Insert sequence 'seq' at position 'pos'
    assert(pos >= 0 && pos <= length_);

    int new_length = length_ + seq_length;
    char* new_seq = new char[new_length];

    for (auto i = 0; i < pos; i++) {
        new_seq[i] = seq_[i];
    }

    for (auto i = pos; i < pos + seq_length; i++) {
        new_seq[i] = seq[i - pos];
    }

    for (auto i = pos + seq_length; i < new_length; i++) {
        new_seq[i] = seq_[i - seq_length];
    }
    
    length_ = new_length;
    seq_ = new_seq;
}

void Dna::do_switch(int pos) {
    if (seq_[pos] == '0') seq_[pos] = '1';
    else seq_[pos] = '0';
}

void Dna::do_duplication(int pos_1, int pos_2, int pos_3) {
    // Duplicate segment [pos_1; pos_2[ and insert the duplicate before pos_3
    char *seq_dupl = NULL;

    int length_dupl;

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

        length_dupl = pos_2 - pos_1;
        seq_dupl = new char[length_dupl];
        for (auto i = pos_1; i < pos_2; i++) {
            seq_dupl[i] = seq_[i];
        }

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

        int length_tmp1 = length_ - pos_1;
        int length_tmp2 = pos_2;
        length_dupl = length_tmp1 + length_tmp2;

        seq_dupl = new char[length_dupl];
        for (auto i = 0; i < length_tmp1; i++) {
            seq_dupl[i] = seq_[pos_1 + i];
        }
        for (auto i = length_tmp1; i < length_dupl; i++) {
            seq_dupl[i] = seq_[i - length_tmp1];
        }

    }

    insert(pos_3, seq_dupl, length_dupl);

}

int Dna::promoter_at(int pos) {
    int prom_dist[PROM_SIZE];

    for (int motif_id = 0; motif_id < PROM_SIZE; motif_id++) {
        int search_pos = pos + motif_id;
        if (search_pos >= length_)
            search_pos -= length_;
        // Searching for the promoter
        prom_dist[motif_id] =
                PROM_SEQ[motif_id] == seq_[search_pos] ? 0 : 1;

    }


    // Computing if a promoter exists at that position
    int dist_lead = prom_dist[0] +
                    prom_dist[1] +
                    prom_dist[2] +
                    prom_dist[3] +
                    prom_dist[4] +
                    prom_dist[5] +
                    prom_dist[6] +
                    prom_dist[7] +
                    prom_dist[8] +
                    prom_dist[9] +
                    prom_dist[10] +
                    prom_dist[11] +
                    prom_dist[12] +
                    prom_dist[13] +
                    prom_dist[14] +
                    prom_dist[15] +
                    prom_dist[16] +
                    prom_dist[17] +
                    prom_dist[18] +
                    prom_dist[19] +
                    prom_dist[20] +
                    prom_dist[21];

    return dist_lead;
}

// Given a, b, c, d boolean variable and X random boolean variable,
// a terminator look like : a b c d X X !d !c !b !a
int Dna::terminator_at(int pos) {
    int term_dist[TERM_STEM_SIZE];
    for (int motif_id = 0; motif_id < TERM_STEM_SIZE; motif_id++) {
        int right = pos + motif_id;
        int left = pos + (TERM_SIZE - 1) - motif_id;

        // loop back the dna inf needed
        if (right >= length_) right -= length_;
        if (left >= length_) left -= length_;

        // Search for the terminators
        term_dist[motif_id] = seq_[right] != seq_[left] ? 1 : 0;
    }
    int dist_term_lead = term_dist[0] +
                         term_dist[1] +
                         term_dist[2] +
                         term_dist[3];

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

        if (seq_[t_pos] == SHINE_DAL_SEQ[k_t]) {
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

        if (seq_[t_k] == PROTEIN_END[k]) {
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
        if (seq_[t_pos] == '1')
            value += 1 << (CODON_SIZE - i - 1);
    }

    return value;
}