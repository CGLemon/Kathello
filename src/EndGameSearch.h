#include <vector>
#include <memory>
#include "Cache.h"
#include "Board.h"
#include "GameState.h" 

class MaxminNode {
public:
    MaxminNode(const int vertex);
    bool expend_children(GameState &state);

    MaxminNode *get_node();
    MaxminNode *get_best_child();

    std::vector<std::shared_ptr<MaxminNode>>& get_children();

    int get_vertex() const;
    int get_score(int color) const;

    bool score_node(GameState &state);
    void set_score(const int score, const int color);
    void set_bestvertex(const int vtx);

private:
    std::vector<std::shared_ptr<MaxminNode>> m_children;
    int m_vertex;
    int m_color{Board::INVAL};

    int m_black_score{0};
    int m_best_vertex{-1};
};

class EndGameSearch {
public:
    struct Result {
        std::vector<int> ownership;
        float score_with_komi;
        int score;
    };

    EndGameSearch(GameState &state, int empty_cnt);

    Result search();

    bool valid() const;

    std::vector<int> get_pv(MaxminNode *root);

private:
    int loop(GameState &current_state, MaxminNode *current_node);

    GameState m_rootstate;
    MaxminNode *m_rootnode{nullptr};
    bool m_allow_search{false};
};

struct EndGameCache {
    Cache<EndGameSearch::Result> m_cache;
    void resize(const int cache_size);
    void clear();
    bool prob_cache(const GameState *const state,
                    EndGameSearch::Result &result);
    void insert_cache(const GameState *const state,
                      EndGameSearch::Result &result);
};
