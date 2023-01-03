//
// Created by arrouan on 01/10/18.
//

#pragma once

#include <zlib.h>

#include "Threefry.h"
#include "aevol_constants.h"

class Dna {

public:
    Dna() = default;

    Dna(const Dna &clone);

    Dna(int length, Threefry::Gen &&rng);

    ~Dna() = default;

    void save(gzFile backup_file);

    void load(gzFile backup_file);

    void set(int pos, char c);

    /// Remove the DNA inbetween pos_1 and pos_2
    void remove(int pos_1, int pos_2);

    /// Insert a sequence of a given length at a given position into the DNA of the Organism
    void insert(int pos, const char* seq, int seq_length);

    void do_switch(int pos);

    void do_duplication(int pos_1, int pos_2, int pos_3);

    int promoter_at(int pos);

    int terminator_at(int pos);

    bool shine_dal_start(int pos);

    bool protein_stop(int pos);

    int codon_at(int pos);

    char* seq_;
    int length_;
};
