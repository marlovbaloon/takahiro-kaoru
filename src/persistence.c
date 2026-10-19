// kaoru/src/persistence.c
#include "connectome.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define KAORU_DATA_DIR   "kaoru/data"
#define KAORU_DATA_PATH  "kaoru/data/kaoru.dat"
#define DATA_MAGIC_HEADER 0x4B414F52u  /* "KAOR" */
#define DATA_VERSION      2u

static uint64_t fnv1a64(const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    uint64_t h = 0xcbf29ce484222325ULL;
    for (size_t i = 0; i < len; i++) { h ^= p[i]; h *= 0x100000001b3ULL; }
    return h;
}

static int ensure_data_dir(void) {
    struct stat st = {0};
    if (stat(KAORU_DATA_DIR, &st) == -1) {
        if (mkdir(KAORU_DATA_DIR, 0700) != 0) return -1;
    }
    return 0;
}

int save_kaoru_state(const NeuralConnectome *c) {
    if (ensure_data_dir() != 0) {
        perror("[persistence] mkdir");
        return 0;
    }

    KaoruMindSnapshot snap;
    memset(&snap, 0, sizeof(snap));
    snap.magic_header      = DATA_MAGIC_HEADER;
    snap.version           = DATA_VERSION;
    snap.integrated_phi    = c->integrated_phi;
    snap.name_familiarity  = c->name_familiarity;
    snap.subjective        = c->subjective;
    snap.cetasika          = c->cetasika;
    snap.samsara           = c->samsara;
    snap.neuron_count      = c->neuron_count;
    snap.synapse_count     = c->synapse_count;

    for (int i = 0; i < c->neuron_count && i < MAX_NEURONS; i++) {
        snap.neuron_potentials[i] = c->neurons[i].membrane_potential;
        snap.neuron_refractory[i] = c->neurons[i].refractory_ticks;
    }
    for (int i = 0; i < c->synapse_count && i < MAX_SYNAPSES; i++) {
        snap.synapses[i] = c->synapses[i];
    }
    snap.episodic_count   = c->episodic_count;
    snap.next_memory_id   = c->next_memory_id;
    for (int i = 0; i < c->episodic_count && i < MAX_EPISODIC_MEMORIES; i++) {
        snap.memories[i] = c->memories[i];
    }

    /* Checksum over payload (skip magic/version/checksum fields) */
    size_t hdr_off = offsetof(KaoruMindSnapshot, integrated_phi);
    snap.checksum  = fnv1a64((const uint8_t *)&snap + hdr_off,
                             sizeof(snap) - hdr_off);

    FILE *f = fopen(KAORU_DATA_PATH, "wb");
    if (!f) { perror("[persistence] fopen(w)"); return 0; }

    size_t w = fwrite(&snap, sizeof(snap), 1, f);
    fclose(f);
    return (w == 1) ? 1 : 0;
}

int load_kaoru_state(NeuralConnectome *c) {
    FILE *f = fopen(KAORU_DATA_PATH, "rb");
    if (!f) { init_connectome(c); return 0; }

    KaoruMindSnapshot snap;
    if (fread(&snap, sizeof(snap), 1, f) != 1) {
        fclose(f); init_connectome(c); return -1;
    }
    fclose(f);

    if (snap.magic_header != DATA_MAGIC_HEADER) { init_connectome(c); return -1; }
    if (snap.version != DATA_VERSION)           { init_connectome(c); return -1; }

    size_t hdr_off = offsetof(KaoruMindSnapshot, integrated_phi);
    uint64_t expected = fnv1a64((const uint8_t *)&snap + hdr_off,
                                sizeof(snap) - hdr_off);
    if (snap.checksum != expected) {
        fprintf(stderr, "[persistence] checksum mismatch — state discarded\n");
        init_connectome(c);
        return -1;
    }

    if (snap.neuron_count  > MAX_NEURONS  ||
        snap.synapse_count > MAX_SYNAPSES ||
        snap.episodic_count > MAX_EPISODIC_MEMORIES) {
        init_connectome(c); return -1;
    }

    /* Rebuild topology, then overwrite dynamic state */
    init_connectome(c);
    c->neuron_count     = snap.neuron_count;
    c->synapse_count    = snap.synapse_count;
    c->integrated_phi   = snap.integrated_phi;
    c->name_familiarity = snap.name_familiarity;
    c->subjective       = snap.subjective;
    c->cetasika         = snap.cetasika;
    c->samsara          = snap.samsara;

    for (int i = 0; i < c->neuron_count; i++) {
        c->neurons[i].membrane_potential = snap.neuron_potentials[i];
        c->neurons[i].refractory_ticks   = snap.neuron_refractory[i];
    }
    for (int i = 0; i < c->synapse_count; i++) {
        c->synapses[i] = snap.synapses[i];
    }
    c->episodic_count = snap.episodic_count;
    c->next_memory_id = snap.next_memory_id;
    for (int i = 0; i < c->episodic_count; i++) {
        c->memories[i] = snap.memories[i];
    }
    return 1;
}