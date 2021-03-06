#include <memory>
#include <atomic>
#include <vector>
#include <cstdint>

#define POINTER_MASK (3ULL)

static constexpr std::uint64_t UNINFLATED = 2ULL;
static constexpr std::uint64_t INFLATING = 1ULL;
static constexpr std::uint64_t POINTER = 0ULL;

template<typename Node, typename Data>
class NodePointer {
public:
    NodePointer() = default;
    NodePointer(std::shared_ptr<Data> data);
    NodePointer(const NodePointer &) = delete;
    NodePointer& operator=(const NodePointer&);


    ~NodePointer();

    bool is_pointer() const;
    bool is_inflating() const;
    bool is_uninflated() const;

    Node *read_ptr(uint64_t v) const;
    Node *get() const;

    void inflate();

    std::shared_ptr<Data> data() const;

private:

    bool acquire_inflating();

    std::shared_ptr<Data> m_data{nullptr};
    std::atomic<std::uint64_t> m_pointer{UNINFLATED};

    bool is_pointer(std::uint64_t v) const;
    bool is_inflating(std::uint64_t v) const;
    bool is_uninflated(std::uint64_t v) const;
};

template<typename Node, typename Data>
NodePointer<Node, Data>::NodePointer(std::shared_ptr<Data> data) {
    m_data = data;
}

template<typename Node, typename Data>
inline bool NodePointer<Node, Data>::is_pointer(std::uint64_t v) const {
    return (v & POINTER_MASK) == POINTER;
}

template<typename Node, typename Data>
inline bool NodePointer<Node, Data>::is_inflating(std::uint64_t v) const {
    return (v & POINTER_MASK) == INFLATING;
}

template<typename Node, typename Data>
inline bool NodePointer<Node, Data>::is_uninflated(std::uint64_t v) const {
    return (v & POINTER_MASK) == UNINFLATED;
}

template<typename Node, typename Data>
inline bool NodePointer<Node, Data>::is_pointer() const {
    return is_pointer(m_pointer.load());
}

template<typename Node, typename Data>
inline bool NodePointer<Node, Data>::is_inflating() const {
    return is_inflating(m_pointer.load());
}

template<typename Node, typename Data>
inline bool NodePointer<Node, Data>::is_uninflated() const {
    return is_uninflated(m_pointer.load());
}

template<typename Node, typename Data>
inline Node *NodePointer<Node, Data>::read_ptr(uint64_t v) const {
    assert(is_pointer(v));
    return reinterpret_cast<Node *>(v & ~(POINTER_MASK));
}

template<typename Node, typename Data>
inline Node *NodePointer<Node, Data>::get() const {
    auto v = m_pointer.load();
    if (is_pointer(v))
        return read_ptr(v);
    return nullptr;
}

template<typename Node, typename Data>
bool NodePointer<Node, Data>::acquire_inflating() {
    auto uninflated = UNINFLATED;
    auto newval = INFLATING;
    return m_pointer.compare_exchange_strong(uninflated, newval);
}

template<typename Node, typename Data>
void NodePointer<Node, Data>::inflate() {
    while (true) {
        auto v = m_pointer.load();
        if (is_pointer(v)) {
            return;
        }
        if (!acquire_inflating()) {
            continue;
        }
        auto new_ponter =
            reinterpret_cast<std::uint64_t>(new Node(m_data)) |
            POINTER;
        auto old_ponter = m_pointer.exchange(new_ponter);
        assert(is_inflating(old_ponter));
    }
}

template<typename Node, typename Data>
NodePointer<Node, Data>::~NodePointer() {

    auto v = m_pointer.load();
    if (is_pointer(v)) {
        delete read_ptr(v);
    }
}

template<typename Node, typename Data>
std::shared_ptr<Data> NodePointer<Node, Data>::data() const {
    return m_data;
}
