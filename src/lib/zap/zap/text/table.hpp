#include <tabulate/table.hpp>

namespace zap::text {

class table
{
public:
    template <typename... Args>
    table(Args&&... args)
    {
        pre();
        t_.add_row({ std::forward<Args>(args)... });
    }

    virtual ~table();

    template <typename... Args>
    void add_row(Args&&... args)
    {
        ++rows_;
        t_.add_row({ std::forward<Args>(args)... });
    }

private:
    friend std::ostream& operator<<(std::ostream& os, table& t);

    void pre();
    void post();

    tabulate::Table t_;
    std::size_t rows_ = 0;
};

inline
std::ostream&
operator<<(std::ostream& os, table& t)
{
    t.post();

    os << t.t_;

    return os;
}

}
