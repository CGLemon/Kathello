#ifndef BOARD_H_INCLUDE
#define BOARD_H_INCLUDE

#include <array>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <string>
#include <sstream>
#include <iostream>
#include <functional>
#include <algorithm>
#include <cstring>

#include "Zobrist.h"
#include "config.h"

#define BLACK_NUMBER (0)
#define WHITE_NUMBER (1)
#define EMPTY_NUMBER (2)
#define INVAL_NUMBER (3)

#define NBR_SHIFT (4)
#define BLACK_NBR_SHIFT (BLACK_NUMBER * 4)
#define WHITE_NBR_SHIFT (WHITE_NUMBER * 4)
#define EMPTY_NBR_SHIFT (EMPTY_NUMBER * 4)

#define NBR_MASK (0xf)
#define BLACK_EYE_MASK (4 * (1 << BLACK_NBR_SHIFT))
#define WHITE_EYE_MASK (4 * (1 << WHITE_NBR_SHIFT))

class Board {
public:
    enum vertex_t : std::uint8_t {
        BLACK = BLACK_NUMBER,
        WHITE = WHITE_NUMBER,
        EMPTY = EMPTY_NUMBER,
        INVAL = INVAL_NUMBER
    };

    static constexpr int NO_INDEX = NUM_INTERSECTIONS+1;

    static constexpr int NO_VERTEX = 0;

    static constexpr int PASS = NUM_VERTICES + 1;

    static constexpr int RESIGN = NUM_VERTICES + 2;

    static constexpr int NUM_SYMMETRIES = 8;

    static constexpr int IDENTITY_SYMMETRY = 0;

    static constexpr int s_eyemask[2] = {BLACK_EYE_MASK, WHITE_EYE_MASK};

    static std::array<std::array<int, NUM_INTERSECTIONS>, NUM_SYMMETRIES> symmetry_nn_idx_table;

    static std::array<std::array<int, NUM_VERTICES>, NUM_SYMMETRIES> symmetry_nn_vtx_table;

    static std::array<int, 8> m_dirs;

    void reset_board(const int boardsize, const float komi);
    void set_komi(const float komi);
    void set_boardsize(int boardsize);
    void reset_board_data(const int boardsize);

    void set_to_move(int color);

    bool is_star(const int x, const int y) const;

    std::string state_to_string(const vertex_t color, bool is_star) const;
    std::string spcaces_to_string(const int times) const;
    std::string columns_to_string(const int bsize) const;

    void info_stream(std::ostream &out) const;
    void hash_stream(std::ostream &out) const;
    void board_stream(std::ostream &out, const int lastmove = NO_VERTEX) const;
    void board_stream(std::ostream &out, const int lastmov, bool is_sgf) const;

    void text_display() const;
    void display_chain();

    std::pair<int, int> get_symmetry(const int x, const int y, const int symmetry,
                                     const int boardsize);
    int get_to_move() const;
    int get_last_move() const;
    int get_vertex(const int x, const int y) const;
    int get_index(const int x, const int y) const;
    static int get_local(const int x, const int y, const int bsize);

    vertex_t get_state(const int vtx) const;
    vertex_t get_state(const int x, const int y) const;
    int get_boardsize() const;
    int get_intersections() const;
    int get_transform_idx(const int idx, const int sym = IDENTITY_SYMMETRY) const;
    int get_transform_vtx(const int vtx, const int sym = IDENTITY_SYMMETRY) const;
    int get_passes() const;
    int get_movenum() const;
    int get_komi_integer() const;
    float get_komi_float() const;
    float get_komi() const;

    int get_numblacks() const;
    int get_numwhites() const;
    int get_numempty() const;

    std::vector<int> get_ownership() const;

    std::uint64_t get_hash() const;

    std::uint64_t calc_hash(int komove, int sym = IDENTITY_SYMMETRY) const;
    std::uint64_t calc_ko_hash(int sym = IDENTITY_SYMMETRY) const;
    std::uint64_t komi_hash(const float komi) const; 

    void set_passes(int val);
    void increment_passes();

    int get_x(const int vtx) const;
    int get_y(const int vtx) const;
    std::pair<int, int> get_xy(const int vtx) const;


    void play_move(const int vtx, const int color);
    void play_move(const int vtx);

    bool is_legal(const int vtx,
                  const int color) const;

    bool is_pass_legal(const int color) const;

    int calc_reach_color(int color) const;
    int calc_reach_color(int color, int spread_color,
                         std::vector<bool>& buf, std::function<int(int)> f_peek) const;
    float area_score(float komi) const;
    int area_distance() const;

    std::string vertex_to_string(const int vertex) const;
    void vertex_stream(std::ostream &out, const int vertex) const;

    void sgf_stream(std::ostream &out,
                   const int vtx, const int color) const;
    void sgf_stream(std::ostream &out) const;

    std::vector<int> get_movelist(const int color) const;

    int  count_boundary(const int vtx) const;
    bool is_side(const int vtx) const;
    bool is_corner(const int vtx) const;

private:
    std::array<vertex_t, NUM_VERTICES> m_state;

    std::uint64_t m_hash{0ULL}; 

    int m_tomove; 
    int m_letterboxsize;
    int m_numvertices;
    int m_intersections;
    int m_boardsize; 
    int m_lastmove; 
    int m_passes;
    int m_movenum;
    int m_komi_integer;
    float m_komi_float;

    bool is_on_board(const int vtx) const;
    void init_symmetry_table(const int boardsize);
    void init_dirs(const int boardsize);
    void init_bitboard(const int numvertices);

    void fix_board();

    void reseve(const int vtx, const int color);
    void update_stone(const int vtx, const int color);

    // uupdate zobrist
    void update_zobrist(const int vtx, const int new_color, const int old_color);
    void update_zobrist_tomove(const int new_color, const int old_color);
    void update_zobrist_pass(const int new_pass, const int old_pass);
    void update_zobrist_komi(const float new_komi, const float old_komi);

    void exchange_to_move();
};

inline std::uint64_t Board::komi_hash(const float komi) const {
    auto k_hash = std::uint64_t{0ULL};
    auto cp = static_cast<double>(komi);
    std::memcpy(&k_hash, &cp, sizeof(double));
    return k_hash;
}

inline int Board::get_vertex(const int x, const int y) const {
    assert(x >= 0 || x < m_boardsize);
    assert(y >= 0 || y < m_boardsize);
    return (y + 1) * m_letterboxsize + (x + 1);
}

inline int Board::get_index(const int x, const int y) const {
    assert(x >= 0 || x < m_boardsize);
    assert(y >= 0 || y < m_boardsize);
    return y * m_boardsize + x;
}

inline int Board::get_local(const int x, const int y, const int bize) {
    return (y + 1) * (bize+2) + (x + 1);
}

inline Board::vertex_t Board::get_state(const int x, const int y) const {
    return m_state[get_vertex(x, y)];
}

inline Board::vertex_t Board::get_state(const int vtx) const {
    return m_state[vtx];
}

inline int Board::get_transform_idx(const int idx, const int sym) const {
    return symmetry_nn_idx_table[sym][idx];
}

inline int Board::get_transform_vtx(const int vtx, const int sym) const {
    return symmetry_nn_vtx_table[sym][vtx];
}


inline void Board::update_stone(const int vtx, const int color){
	assert(color == EMPTY || color == BLACK || color == WHITE);
	const int old_color = m_state[vtx];
	m_state[vtx] = static_cast<vertex_t>(color);

    update_zobrist(vtx, color, old_color);
}


inline void Board::update_zobrist(const int vtx,
                                  const int new_color,
                                  const int old_color) {
    m_hash ^= Zobrist::zobrist[old_color][vtx];
    m_hash ^= Zobrist::zobrist[new_color][vtx];
}

inline void Board::update_zobrist_tomove(const int new_color,
                                         const int old_color) {
    if (old_color != new_color) {
        m_hash ^= Zobrist::zobrist_blacktomove;
    }
}

inline void Board::update_zobrist_pass(const int new_pass,
                                       const int old_pass) {
    m_hash ^= Zobrist::zobrist_pass[old_pass];
    m_hash ^= Zobrist::zobrist_pass[new_pass];
}

inline void Board::update_zobrist_komi(const float new_komi,
                                       const float old_komi) {
    m_hash ^= komi_hash(old_komi);
    m_hash ^= komi_hash(new_komi);
}

inline int Board::get_x(const int vtx) const {
    const int x = (vtx % m_letterboxsize) - 1;
    assert(x >= 0 && x < m_boardsize);
    return x;
}

inline int Board::get_y(const int vtx) const {
    const int y = (vtx / m_letterboxsize) - 1;
    assert(y >= 0 && y < m_boardsize);
    return y;
}

inline std::pair<int, int> Board::get_xy(const int vtx) const {
    const int x = get_x(vtx);
    const int y = get_y(vtx);
    return {x, y};
}

inline bool Board::is_on_board(const int vtx) const {
  return m_state[vtx] != INVAL;
}

#endif
