#include "SGFStream.h"
#include "Utils.h"
#include "config.h"

#include <sstream>
#include <fstream>
#include <iomanip>

void SGFStream::save_sgf(std::string filename, GameState &state, bool append) {

    std::ostringstream out;
    sgf_stream(out, state);
  
    std::fstream sgf;
    auto ios_tag = std::ios::out;
    if (append) {
        ios_tag |= std::ios::app;
    }

    sgf.open(filename, ios_tag);

    if (sgf.is_open()) {
        sgf << out.str();
    } else {
        printf("Error opening file\n");
    }
    sgf.close();
}

void SGFStream::sgf_stream(std::ostream &out, GameState &state) {

    auto winner = state.get_winner();
    float score = -1;
    if (state.board.get_passes() >= 2) {
        score = state.final_score();
        if (winner == Board::WHITE) {
            score = (-score);
        }
        assert(score >= 0);
    }

    out << "(";
    out << ";";

    out << "GM[1]";
    out << "FF[4]";

    if (winner == Board::EMPTY) {
        out << "RE[0]";
        assert(score == 0);
    }
    else if (winner != Board::INVAL) {
        out << "RE[";
        if (winner == Board::BLACK) {
            out << "B+";
        } else if (winner == Board::WHITE) {
            out << "W+";
        } else {
            out << "Error";
        }
        if (score < 0.f) {
            out << "Resign";
        } else {
            out << score;
        }
        out << "]";
    }

    out << "AP[";
    out << PROGRAM;
    out <<  "]";

    out << "RU[";
    out << "]"; 
 
    out << "KM[";
    out << state.board.get_komi();
    out << "]";

    out << "SZ[";
    out << state.board.get_boardsize();
    out << "]";

    out << state.get_sgf_string();
    out << ")";
    out << std::endl;
}

