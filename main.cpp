#include "z3++.h"
#include <iostream>
#include <vector>

z3::expr_vector create_vector_with_num_names(z3::context& c, size_t n, std::string name)
{
    name = name + "_";
    z3::expr_vector xs(c);
    for (unsigned i = 0; i < n; i++) {
        xs.push_back(c.int_const((name + std::to_string(i)).c_str()));
    }
    return xs;
}

void set_queens_order(z3::solver& s, const z3::expr_vector& xs, const z3::expr_vector& ys, int board_size)
{
    for (int i = 1; i < xs.size(); i++) {
        s.add(ys[i - 1] * board_size + xs[i - 1] < ys[i] * board_size + xs[i]);
    }
}

void set_board_size_limit(z3::solver& s, const z3::expr_vector& xs, int board_size)
{
    for (const auto& x : xs) {
        s.add(x >= 1 && x <= board_size);
    }
}

void set_values_distinct(z3::solver& s, const z3::expr_vector& xs)
{
    z3::expr_vector t(s.ctx());
    for (const auto& x : xs)
        t.push_back(x);
    s.add(distinct(t));
}

z3::expr get_queens_on_diag_expr(const z3::expr& x1, const z3::expr& y1, const z3::expr& x2, const z3::expr& y2)
{
    return z3::abs(x2 - x1) == z3::abs(y2 - y1);
}

void restrict_one_queen_on_diag(z3::solver& s, const z3::expr_vector& xs, const z3::expr_vector& ys)
{
    for (int i = 0; i < xs.size(); i++)
        for (int j = i + 1; j < ys.size(); j++) {
            s.add(!get_queens_on_diag_expr(xs[i], ys[i], xs[j], ys[j]));
        }
}

std::vector<std::pair<int, int>> get_queens_positions(const z3::model& model, const z3::expr_vector& xs, const z3::expr_vector& ys)
{
    std::vector<std::pair<int, int>> positions;
    for (unsigned i = 0; i < xs.size(); ++i) {
        auto x = model.eval(xs[i]);
        auto y = model.eval(ys[i]);
        positions.emplace_back(x.get_numeral_int(), y.get_numeral_int());
    }
    return positions;
}

void print_queens_positions(const std::vector<std::pair<int, int>>& positions)
{
    for (auto [x, y] : positions) {
        std::cout << static_cast<char>('A' + x - 1) << y << "\n";
    }
}

void print_board(const std::vector<std::pair<int, int>>& positions, int board_size)
{
    std::vector<std::vector<char>> board(board_size, std::vector<char>(board_size, '+'));
    for (auto [x, y] : positions)
        board[y - 1][x - 1] = 'Q';

    for (auto& row : board) {
        for (auto& elem : row)
            std::cout << elem << ' ';
        std::cout << "\n";
    }
}

void add_solution_to_solver(z3::solver& solver, const z3::model& model, const z3::expr_vector& xs, const z3::expr_vector& ys)
{
    z3::expr_vector is_cur_solution(solver.ctx());
    for (unsigned i = 0; i < xs.size(); ++i) {
        auto x = model.eval(xs[i]);
        auto y = model.eval(ys[i]);
        is_cur_solution.push_back(xs[i] == x && ys[i] == y);
    }
    solver.add(!z3::mk_and(is_cur_solution));
}

void queen_solve(int queen_count)
{
    std::cout << "run queen solve for n=" << queen_count << "\n";
    z3::context context;
    auto xs = create_vector_with_num_names(context, queen_count, "x");
    auto ys = create_vector_with_num_names(context, queen_count, "y");

    z3::solver solver(context);

    set_queens_order(solver, xs, ys, queen_count);

    set_board_size_limit(solver, xs, queen_count);
    set_board_size_limit(solver, ys, queen_count);

    set_values_distinct(solver, xs);
    set_values_distinct(solver, ys);
    restrict_one_queen_on_diag(solver, xs, ys);

    auto solutions_count = 0;
    while (solver.check() == z3::sat) {
        auto model = solver.get_model();

        auto positions = get_queens_positions(model, xs, ys);
        std::cout << "\n";
        print_board(positions, queen_count);

        add_solution_to_solver(solver, model, xs, ys);
        solutions_count++;
    }
    std::cout << "solutions count for n = " << queen_count << ": " << solutions_count << "\n\n";
}

int main()
{
    queen_solve(3);
    queen_solve(4);
    queen_solve(8);

    Z3_finalize_memory();
    return 0;
}
