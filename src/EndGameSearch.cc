#include <cassert>
#include "EndGameSearch.h"

MaxminNode::MaxminNode(const int vertex) {
    m_vertex = vertex;
}

bool MaxminNode::expend_children(GameState &state) {

    assert(state.get_passes() < 2);
    if (!m_children.empty()) {
        return false;
    }

    const auto boardsize = state.get_boardsize();
    const auto intersections = state.get_intersections();
    m_color = state.get_to_move();

    auto nodelist = std::vector<int>{};

    for (int i = 0; i < intersections; ++i) {
        const auto x = i % boardsize;
        const auto y = i / boardsize;
        const auto vertex = state.get_vertex(x, y);

        if (state.is_legal(vertex, m_color)) {
            nodelist.emplace_back(vertex);
        }
    }

    if (nodelist.empty() && state.is_legal(Board::PASS, m_color)) {
        nodelist.emplace_back(Board::PASS);
    }

    assert(!nodelist.empty());
    for (const auto &v : nodelist) {
        m_children.emplace_back(std::make_shared<MaxminNode>(v));
    }
    return true;
}

MaxminNode *MaxminNode::get_node() {
    return this;
}

MaxminNode *MaxminNode::get_best_child() {

    if (m_color == Board::INVAL) {
        return nullptr;
    }

    assert(m_best_vertex != -1);
    auto best = m_children[0]->get_node();

    for (const auto &child: m_children) {
        auto vtx = child->get_vertex();
        if (m_best_vertex == vtx) {
            best = child->get_node();
            break;
        }
    }
    return best;
}

std::vector<std::shared_ptr<MaxminNode>>& MaxminNode::get_children() {
    assert(m_color != Board::INVAL);
    return m_children;
}

int MaxminNode::get_vertex() const {
    return m_vertex;
}

int MaxminNode::get_score(int color) const {
    assert(color == Board::BLACK || color == Board::WHITE);
    if (color == Board::BLACK) {
        return m_black_score;
    }
    return 0 - m_black_score;
}

bool MaxminNode::score_node(GameState &state) {

    if (state.get_passes() < 2) {
        return false;
    }
    assert(m_children.empty());
    m_black_score = state.board.area_distance();
    return true;
}

void MaxminNode::set_score(const int score, const int color) {
    assert(color == Board::BLACK || color == Board::WHITE);
    if (color == Board::BLACK) {
        m_black_score = score;
    }
    m_black_score = 0 - score;
}

void MaxminNode::set_bestvertex(const int vtx) {
    m_best_vertex = vtx;
}

EndGameSearch::EndGameSearch(GameState &state, int empty_cnt) {
    m_rootstate = state;

    if (m_rootstate.board.get_numempty() <= empty_cnt) {
        m_allow_search = true;
    }
}

EndGameSearch::Result EndGameSearch::search() {

    assert(m_allow_search);

    m_rootnode = new MaxminNode(Board::PASS);
    
    const auto score = loop(m_rootstate, m_rootnode);
    const auto pv = get_pv(m_rootnode);

    auto state = std::make_shared<GameState>(m_rootstate);
    for (const auto &v: pv) {
        state->play_move(v);
    }

    assert(state->get_passes() == 2);
    const auto score_with_komi = state->final_score();
    const auto ownership = state->board.get_ownership();

    auto result = Result{};
    result.ownership = ownership;
    result.score_with_komi = score_with_komi;
    result.score = score;

    if (m_rootstate.get_to_move() == Board::WHITE) {
        result.score = 0 - result.score;
    }

    delete m_rootnode;
    return result;
}

int EndGameSearch::loop(GameState &current_state, MaxminNode *current_node) {

    if (current_node->score_node(current_state)) {
        const auto color = current_state.get_to_move();
        const auto score = current_node->get_score(color);
        return score;
    }

    const auto intersections = current_state.get_intersections();

    auto best_score = 0 - intersections;
    auto success = current_node->expend_children(current_state);
    assert(success);

    auto children = current_node->get_children();
    for (auto &child: children) {
        auto next_state = current_state;
        auto vtx = child->get_vertex();
        next_state.play_move(vtx);
        auto next_score = 0 - loop(next_state, child->get_node());
        if (next_score > best_score) {
            best_score = next_score;
            current_node->set_bestvertex(vtx);
        }
    }

    const auto color = current_state.get_to_move();
    current_node->set_score(best_score, color);

    return best_score;
}

bool EndGameSearch::valid() const {
    return m_allow_search;
}

std::vector<int> EndGameSearch::get_pv(MaxminNode *root) {

    auto res = std::vector<int>{};
    if (!m_rootnode) {
        return res;
    }

    auto current_node = root->get_best_child();
    while (current_node) {
        const auto vtx = current_node->get_vertex();
        res.emplace_back(vtx);
        current_node = current_node->get_best_child();
    }
    return res;
}

void EndGameCache::resize(const int cache_size) {
    m_cache.resize(cache_size);
}

void EndGameCache::clear() {
    m_cache.clear();
}

bool EndGameCache::prob_cache(const GameState *const state,
                              EndGameSearch::Result &result) {
    return m_cache.lookup(state->board.get_hash(), result);
}

void EndGameCache::insert_cache(const GameState *const state,
                                EndGameSearch::Result &result) {
    m_cache.insert(state->board.get_hash(), result);
}
