#ifndef HEADER_H
#define HEADER_H

#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>

class Back {
public:
    struct information {
        std::string type_id;
        std::string name;
        double a = 0.0;
        double b = 0.0;
        double d = 0.0;
        double feed_norm = 0.0;
        double fish_price = 0.0;
        double feed_price = 0.0;
        double pollution_factor = 0.0;
    };

    struct Fish {
        int young = 0;
        int old = 0;
    };

    struct Prud {
        int id = 0;
        bool active = false;
        bool cleaning = false;
        std::string type_id;
        Fish fish;
        double current_feed = 0.0;
        double pollution = 0.0;
    };

    struct ContractPeriod {
        int from_week = 0;
        int to_week = 0;
        double fish_price = 0.0;
        double feed_price = 0.0;
    };

    struct contract {
        int all_weeks = 0;
        double week_korm = 0.0;
        int need_fish = 0;
        double minus_kg = 0.0;
        std::vector<ContractPeriod> periods;
        bool active = false;
        int curr = 0;
        double current_week_korm = 0.0;
        int sold_fish = 0;
    };

    struct State {
        double money = 0;
        int week = 0;
        bool no_money = false;
        std::vector<Prud> pruds;
        contract contracts_svo;
        int totalFish = 0;
    };

    Back();

    std::vector<information> get_fish_type();
    void load_fish_from_file(const std::string& filename);
    void start_all(int count_prud, double money);
    void nextWeek();

    void new_prud(int idi, const std::string& type_id);
    void delete_prud(int idi);
    void start_cleaning(int idi);

    void add_fish(int idi, int young, int old);
    void sell_fish(int idi, int sum);
    void buy_korm(int idi, double sum);

    void createContract(int weeks, double week_korm, int need_fish, double minus_kg, const std::vector<ContractPeriod>& periods);
    void delete_contract();
    void set_loss_range(double min_percent, double max_percent);

    double get_money() const;
    int get_week() const;
    bool get_no_money() const;
    int get_totalFish() const;
    std::vector<Prud> get_pruds() const;
    contract get_contract() const;

private:
    State state;
    std::vector<information> fish_types;
    std::vector<Fish> cleaning_fish_backup;

    std::mt19937 random;

    double loss_min = 0.05;
    double loss_max = 0.10;

    void in_week();
    void prud_do();
    void in_prud(Prud& prud);
    void random_loss(Prud& prud);
    void cleaning_do();
    void do_contract();
    void check_money_0();

    Prud* find(int idi);
    information* find_type(const std::string& type_id);

    void calculate_total_fish();
    double get_current_fish_price() const;
    double get_current_korm_price() const;
};

#endif