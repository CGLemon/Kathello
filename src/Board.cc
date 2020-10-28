#include <array>
#include <memory>
#include <queue>
#include <vector>
#include <iomanip>
#include <algorithm>

#include "Board.h"
#include "Utils.h"
#include "Zobrist.h"
#include "config.h"

using namespace Utils;

constexpr int Board::RESIGN;
constexpr int Board::PASS;
constexpr int Board::NO_VERTEX;
constexpr int Board::NUM_SYMMETRIES;
constexpr int Board::IDENTITY_SYMMETRY;
constexpr int Board::s_eyemask[2];

std::array<std::array<int, NUM_INTERSECTIONS>, Board::NUM_SYMMETRIES>
    Board::symmetry_nn_idx_table;
std::array<std::array<int, NUM_VERTICES>, Board::NUM_SYMMETRIES>
    Board::symmetry_nn_vtx_table;
std::array<int, 8> Board::m_dirs;

std::pair<int, int> Board::get_symmetry(const int x, const int y,
                                        const int symmetry,
                                        const int boardsize) {

    assert(x >= 0 && x < boardsize);
    assert(y >= 0 && y < boardsize);
    assert(symmetry >= 0 && symmetry < NUM_SYMMETRIES);

    int idx_x = x;
    int idx_y = y;

    if ((symmetry & 4) != 0) {
        std::swap(idx_x, idx_y);
    }

    if ((symmetry & 2) != 0) {
        idx_x = boardsize - idx_x - 1;
    }

    if ((symmetry & 1) != 0) {
        idx_y = boardsize - idx_y - 1;
    }

    assert(idx_x >= 0 && idx_x < boardsize);
    assert(idx_y >= 0 && idx_y < boardsize);
    assert(symmetry != IDENTITY_SYMMETRY || (x == idx_x && y == idx_y));

    return {idx_x, idx_y};
}

void Board::init_symmetry_table(const int boardsize) {

    for (int sym = 0; sym < NUM_SYMMETRIES; ++sym) {
        for (int vtx = 0; vtx < NUM_VERTICES; ++vtx) {
            symmetry_nn_vtx_table[sym][vtx] = 0;
        }
        for (int idx = 0; idx < NUM_INTERSECTIONS; ++idx) {
            symmetry_nn_idx_table[sym][idx] = 0;
        }
    }

    for (int sym = 0; sym < NUM_SYMMETRIES; ++sym) {
        for (int y = 0; y < boardsize; y++) {
            for (int x = 0; x < boardsize; x++) {
                const auto sym_idx = get_symmetry(x, y, sym, boardsize);
                const auto vtx = get_vertex(x, y);
                const auto idx = get_index(x, y);
                symmetry_nn_idx_table[sym][idx] =
                    get_index(sym_idx.first, sym_idx.second);
                symmetry_nn_vtx_table[sym][vtx] =
                    get_vertex(sym_idx.first, sym_idx.second);
            }
        }
    }
}

void Board::init_dirs(const int boardsize) {
    const int x_shift = boardsize + 2;
    m_dirs[0] = (-x_shift);
    m_dirs[1] = (-1);
    m_dirs[2] = (+1);
    m_dirs[3] = (+x_shift);
    m_dirs[4] = (-x_shift - 1);
    m_dirs[5] = (-x_shift + 1);
    m_dirs[6] = (+x_shift - 1);
    m_dirs[7] = (+x_shift + 1);
}

void Board::fix_board() {

	const int part_center = m_boardsize / 2;
	
	m_state[get_vertex(part_center,part_center)] = WHITE;
	m_state[get_vertex(part_center-1,part_center-1)] = WHITE;
	m_state[get_vertex(part_center-1,part_center)] = BLACK;
	m_state[get_vertex(part_center,part_center-1)] = BLACK;
}


void Board::reset_board(const int boardsize, const float komi) {
    set_boardsize(boardsize);
    set_komi(komi);
    reset_board_data(m_boardsize);

    m_lastmove = NO_VERTEX;
    m_tomove = BLACK;
    m_passes = 0;
    m_movenum = 0;

    init_symmetry_table(m_boardsize);
    init_dirs(m_boardsize);

    m_hash = calc_hash(NO_VERTEX);
    fix_board();
}

void Board::set_boardsize(int boardsize) {
    if (boardsize > BOARD_SIZE || boardsize < MARCO_MINIMAL_GTP_BOARD_SIZE) {
        boardsize = BOARD_SIZE;
    }
    m_boardsize = boardsize;
    m_letterboxsize = m_boardsize + 2;
    m_numvertices = m_letterboxsize * m_letterboxsize;
    m_intersections = m_boardsize * m_boardsize;
}

void Board::set_komi(const float komi) {

    const auto old_komi = get_komi();
    m_komi_integer = static_cast<int>(komi);
    m_komi_float = komi - static_cast<float>(m_komi_integer);
    if (m_komi_float < 0.01f && m_komi_float > (-0.01f)) {
        m_komi_float = 0.0f;
    }

    update_zobrist_komi(komi, old_komi);
}

void Board::reset_board_data(const int boardsize) {
	for (auto i = 0 ; i < NUM_VERTICES ; ++i){
		m_state[i] 	= INVAL;
	}

	for (auto y = 0 ; y < boardsize ; ++y){
		for (auto x = 0; x < boardsize ; ++x){
			const int vertex = get_vertex(x,y);
			m_state[vertex] = EMPTY;
		}
	}
}

void Board::set_passes(int val) {

     if (val > 4) {
        val = 4;
     }
     update_zobrist_pass(val, m_passes);
     m_passes = val;
}

void Board::increment_passes() {

    int ori_passes = m_passes;
    m_passes++;
    if (m_passes > 4)
        m_passes = 4;
    update_zobrist_pass(m_passes, ori_passes);
}

bool Board::is_star(const int x, const int y) const {

    const int size = m_boardsize;
    const int point = get_index(x, y);
    int stars[3];
    int points[2];
    int hits = 0;

    if (size % 2 == 0 || size < 9) {
        return false;
    }

    stars[0] = size >= 13 ? 3 : 2;
    stars[1] = size / 2;
    stars[2] = size - 1 - stars[0];

    points[0] = point / size;
    points[1] = point % size;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            if (points[i] == stars[j]) {
                hits++;
            }
        }
    }

    return hits >= 2;
}

void Board::info_stream(std::ostream &out) const {

    out << "{";
    out << "Next Player : ";
    if (m_tomove == Board::BLACK) {
        out << "Black";
    } else if (m_tomove == Board::WHITE) {
        out << "White";
    } else {
        out << "Error";
    }
    out << ", ";
    out << "Board Size : "  << m_boardsize              << ", ";
    out << "Komi : "        << get_komi()               << ", ";
    // out << "Label Komi : "   << std::setw(2) << cfg_lable_komi           << ", ";
    // out << "Label Buffer : " << std::setw(2) << cfg_label_buffer * 100.f << "%";
    out << "}\n";
}

void Board::hash_stream(std::ostream &out) const {

    out << std::hex;
    out << "HASH : " << m_hash;
    out << " | ";
    out << "\n";
    out << std::dec;
}

void Board::board_stream(std::ostream &out, const int lastmove, bool is_sgf) const {

    m_boardsize > 9 ? (out << spcaces_to_string(3))
                    : (out << spcaces_to_string(2));
    out << columns_to_string(m_boardsize);
    for (int y = 0; y < m_boardsize; y++) {
    
        const int row = (is_sgf ? y : m_boardsize - y - 1);

        out << std::to_string(row + 1);
        if (row < 9 && m_boardsize > 9) {
            out << spcaces_to_string(1);
        }
        if (lastmove == get_vertex(0, row)) {
            out << "(";
        } else {
            out << spcaces_to_string(1);
        }

        for (int x = 0; x < m_boardsize; x++) {
            const int vtx = get_vertex(x, row);
            const auto state = get_state(vtx);
            out << state_to_string(
                       static_cast<vertex_t>(state), is_star(x, row));

            if (lastmove == get_vertex(x, row)) {
                out << ")";
            } else if (x != m_boardsize - 1 && lastmove == get_vertex(x, row) + 1) {
                out << "(";
            } else {
                out << spcaces_to_string(1);
            }
        }
        out << std::to_string(row + 1);
        out << "\n";
    }
    m_boardsize > 9 ? (out << spcaces_to_string(3))
                    : (out << spcaces_to_string(2));
    out << columns_to_string(m_boardsize);
}

void Board::board_stream(std::ostream &out, const int lastmove) const {
    board_stream(out, lastmove, true);
}

std::string Board::columns_to_string(const int bsize) const {

    auto res = std::string{};
    for (int i = 0; i < bsize; i++) {
        if (i < 25) {
            res += (('a' + i < 'i') ? 'a' + i : 'a' + i + 1);
        } else {
            res += (('A' + (i - 25) < 'I') ? 'A' + (i - 25) : 'A' + (i - 25) + 1);
        }
        res += " ";
    }
    res += "\n";
    return res;
}

std::string Board::state_to_string(const vertex_t color, bool is_star) const {

    auto res = std::string{};
    color == BLACK  ? res += "x" : 
    color == WHITE  ? res += "o" : 
    is_star == true ? res += "+" :
    color == EMPTY  ? res += "." : 
    color == INVAL  ? res += "-" : res += "error";
 
    return res;
}

std::string Board::spcaces_to_string(const int times) const {
    auto res = std::string{};
    for (int i = 0; i < times; ++i) {
        res += " ";
    }
    return res;
}

void Board::text_display() const {

    auto out = std::ostringstream{};
    board_stream(out, m_lastmove);
  
    auto res = out.str();
    auto_printf("%s\n", res.c_str());
}

std::uint64_t Board::calc_hash(int komove, int sym) const {

    std::uint64_t res = calc_ko_hash(sym);
    res ^= Zobrist::zobrist_ko[get_transform_vtx(komove, sym)];
    res ^= Zobrist::zobrist_pass[m_passes];
    if (m_tomove == BLACK) {
        res ^= Zobrist::zobrist_blacktomove;
    }
    return res;
}

std::uint64_t Board::calc_ko_hash(int sym) const {

    std::uint64_t res = Zobrist::zobrist_empty;
    res ^= komi_hash(get_komi());
    for (int vtx = 0; vtx < m_numvertices; vtx++) {
        if (is_on_board(vtx)) {
            res ^= Zobrist::zobrist[m_state[vtx]][get_transform_vtx(vtx, sym)];
        }
    }
    return res;
}

void Board::play_move(const int vtx) {
    play_move(vtx, m_tomove);
}

void Board::reseve(const int vtx, const int color) {

	const auto opp_color = !color;
	auto update_vtx = std::vector<int>{};
	update_stone(vtx, color);

	for(int k = 0; k < 8; ++k){
		int avtx = vtx;
		int res = 0;
		
		do {
			avtx += m_dirs[k];
			res ++;
		} while (m_state[avtx] == opp_color);
		
		if(res > 1 && m_state[avtx] == color){
			for (int i = 0; i < (res-1); ++i){
				avtx -= m_dirs[k];
				update_stone(avtx, color);
				update_vtx.emplace_back(avtx);
			}
		}
	}
}


// assume the move is legal
void Board::play_move(const int vtx, const int color) {

    assert(vtx != Board::RESIGN);
    set_to_move(color);

    if (vtx == PASS) {
        increment_passes();
    } else {
        if (get_passes() != 0) {
            set_passes(0);
        }
        reseve(vtx, color);
    }

    m_lastmove = vtx;
    m_movenum++;
    exchange_to_move();
}

bool Board::is_pass_legal(const int color) const {
    for (int vtx = 0 ; vtx < m_numvertices; ++vtx){
		if (m_state[vtx] == EMPTY) {
			if (is_legal(vtx, color)) {
				return false;
			}
		}
	}
    return true;
}

bool Board::is_legal(const int vtx,
                     const int color) const {

    if (vtx == RESIGN) {
	    return true;
    }

    if (vtx == PASS) {
        return is_pass_legal(color);
    }

	const auto opp_color = !color;
	if(m_state[vtx] == EMPTY){	
		for(auto k = 0; k < 8; ++k){
			int avtx = vtx;
			do {
				avtx += m_dirs[k];
			} while (m_state[avtx] == opp_color);
			
			if (avtx != (vtx + m_dirs[k]) && m_state[avtx] == color) {
				return true;
			}
		}
	}
	return false;
}

int Board::calc_reach_color(int color) const {

    auto res = 0;
    for (int vtx = 0 ; vtx < m_numvertices; ++vtx){
		if (m_state[vtx] == color) {
            res++;
		}
	}
    return res;
}

float Board::area_score(float komi) const {

    const auto white = calc_reach_color(WHITE);
    const auto black = calc_reach_color(BLACK);
    return static_cast<float>(black - white) - komi;
}

int Board::area_distance() const {

    const auto white = calc_reach_color(WHITE);
    const auto black = calc_reach_color(BLACK);
    return black - white;
}

void Board::set_to_move(int color) {
    assert(color == BLACK || color == WHITE);
    update_zobrist_tomove(color, m_tomove);
    m_tomove = color;
}

void Board::exchange_to_move() {
    m_tomove = !(m_tomove);
    update_zobrist_tomove(BLACK, WHITE);
}

int Board::get_boardsize() const {
    return m_boardsize;
}

int Board::get_intersections() const {
    return m_intersections;
}

int Board::get_komi_integer() const {
    return m_komi_integer;
}

float Board::get_komi_float() const {
    return m_komi_float;
}

float Board::get_komi() const {
    return m_komi_float + static_cast<float>(m_komi_integer);
}

int Board::get_to_move() const { 
    return m_tomove; 
}

int Board::get_last_move() const { 
    return m_lastmove;
}

std::uint64_t Board::get_hash() const {
    return m_hash;
}

int Board::get_passes() const {
   return m_passes;
}

int Board::get_movenum() const {
  return m_movenum;
}

std::vector<int> Board::get_ownership() const {

    auto res = std::vector<int>(m_intersections, INVAL);
    for (int y = 0; y < m_boardsize; ++y) {
        for (int x = 0; x < m_boardsize; ++x) {
            const auto idx = get_index(x, y);
            const auto vtx = get_vertex(x, y);
            const auto color = m_state[vtx];
            assert(color != Board::INVAL);
            res[idx] = color;
       }
    }

    return res;
}

void Board::vertex_stream(std::ostream &out, const int vertex) const {

    assert(vertex != NO_VERTEX);

    if (vertex == PASS) {
       out << "pass";
       return;
    } else if (vertex == RESIGN) {
       out << "resign";
       return;
    }
    const auto x = get_x(vertex);
    const auto y = get_y(vertex);
    auto x_char = static_cast<char>(x + 65);
    if (x_char >= 'I') {
        x_char++;
    }
    auto y_str = std::to_string(y + 1);

    out << x_char;
    out << y_str;
}

std::string Board::vertex_to_string(const int vertex) const {
    auto res = std::ostringstream{};
    vertex_stream(res, vertex);
    return res.str();
}

void Board::sgf_stream(std::ostream &out,
                      const int vertex, const int color) const {
    assert(vertex != NO_VERTEX);
  
    if (color == BLACK) {
        out << "B";
    } else if (color == WHITE) {
        out << "W";
    } else {
        out << "error";
    }

    if (vertex == PASS || vertex == RESIGN) {
        out << "[]";
    } else if (vertex > NO_VERTEX && vertex < m_numvertices){
        const auto x = get_x(vertex);
        const auto y = get_y(vertex);

        const auto x_char = static_cast<char>(x +
                                (x < 25 ? 'a' : 'A'));
        const auto y_char = static_cast<char>(y +
                                (y < 25 ? 'a' : 'A'));

        out << '[';
        out << x_char << y_char;
        out << ']';
    } else {
        out << "[error]";
    }
}

void Board::sgf_stream(std::ostream &out) const {
    sgf_stream(out, m_lastmove, !(m_tomove));
}

std::vector<int> Board::get_movelist(const int color) const {

    auto movelist = std::vector<int>{};
    for (int vtx = 0 ; vtx < m_numvertices; ++vtx){
		if (m_state[vtx] == EMPTY) {
			if (is_legal(vtx, color)) {
				movelist.emplace_back(vtx);
			}
		}
	}

    if (movelist.empty()) {
        movelist.emplace_back(Board::PASS);
    }

    return movelist;
}

int Board::count_boundary(const int vtx) const {

    if (m_state[vtx] == INVAL) {
        return 0;
    }

    int res = 0;
    for (int k = 0; k < 4; ++k) {
        const auto avtx = vtx + m_dirs[k];
        if (m_state[avtx] == INVAL) {
            res++;
        }
    }
    return res;
}

bool Board::is_side(const int vtx) const {
    return count_boundary(vtx) == 1; 
}

bool Board::is_corner(const int vtx) const {
    return count_boundary(vtx) == 2; 
}



