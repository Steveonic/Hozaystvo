#include "Header.h"
#include <algorithm>
#include <cmath>
//

using namespace std;

Back::Back(const Funck& callbacks) : fanction(callbacks), random(std::random_device{}()) {
    state = State{};
    state.money = 0.0;
    state.week = 0;
    state.no_money = false;
    state.totalFish = 0;
    state.pruds.clear();
    korm.clear();
    not_active_prud.clear();
}

std::vector<Back::information> Back::get_fish_type() {
    vector <Back::information> aap(7);

    aap[0] = { Typer::Trout,L"Форель", 2.0, 0.8,0.05,1.2,450.0, 80.0 };

    aap[1] = { Typer::Carp,L"Карп", 3.0, 0.75, 0.04,1.0,300.0, 60.0 };

    aap[2] = { Typer::Pike,L"Щука",1.5,0.7, 0.07, 1.5,  500.0, 100.0 };
    aap[3] = { Typer::Catfish,L"Сом",2.0,0.72,0.06,1.4,400.0,90.0 };
    aap[4] = { Typer::Perch,L"Окунь", 2.5,0.78,0.05,0.9,280.0,55.0 };
    aap[5] = { Typer::Crucian, L"Карась",  3.5, 0.82, 0.03, 0.8,220.0,45.0 };
    aap[6] = { Typer::SilverCarp, L"Толстолобик", 2.8, 0.76, 0.04, 1.1, 350.0, 70.0 };
    return aap;
}

Back::information Back::get_information(Typer type) {
    const auto fishes = get_fish_type();
    const auto i = find_if(fishes.begin(), fishes.end(), [type](const information& fish) {
        return fish.type == type;
        });

    if (i != fishes.end())
        return *i;

    return{ Typer::None,L"Нет рыбы",0.0,0.0,0.0,0.0,0.0,0.0 };
}


void Back::start_all(int count_prud, double first_money) {
    state = State{};

    state.money = first_money;
    state.week = 0;
    state.no_money = false;
    state.totalFish = 0;

    state.pruds.clear();
    korm.clear();
    not_active_prud.clear();

    for (int i = 0; i < count_prud; ++i) {
        Prud prudic;

        prudic.id = i;
        prudic.active = false;
        prudic.cleaning = false;

        state.pruds.push_back(prudic);
        korm.push_back(0.0);
    }
    cout << 11;
    about();
}

Back::Prud* Back::find_prud(int idi) {
    auto x = std::find_if(state.pruds.begin(), state.pruds.end(), [idi](const Prud& prudic) {
        return prudic.id == idi;
        });

    if (x == state.pruds.end())
        return nullptr;
    return &(*x);
}


const Back::Prud* Back::find(int idi) const {
    auto x = std::find_if(state.pruds.begin(), state.pruds.end(), [idi](const Prud& prudic) {
        return prudic.id == idi;
        });
    if (x == state.pruds.end())
        return nullptr;

    return &(*x);
}


bool Back::new_prud(int idi, Typer type) {
    Prud* prudic = find_prud(idi);

    if (!prudic)
        return false;

    if (prudic->active)
        return false;

    information info = get_information(type);

    if (info.type == Typer::None)
        return false;

    prudic->active = true;
    prudic->cleaning = false;

    prudic->type = info.type;

    prudic->fish = Fish{};

    prudic->a = info.a;
    prudic->b = info.b;
    prudic->d = info.d;
    prudic->korm_norm = info.korm_normal;
    prudic->price = info.price;
    prudic->korm_price = info.korm_price;
    return true;
}


bool Back::delete_prud(int idi) {
    Prud* prudic = find_prud(idi);
    /*for (auto& i : state.pruds) {
        if (i.id == idi) {

            break;
        }
    }*/
    if (!prudic)
        return false;

    if (!prudic->active)
        return false;

    if (prudic->cleaning)
        return false;

    prudic->active = false;
    prudic->type = Typer::None;
    prudic->fish = Fish{};

    prudic->a = 0.0;
    prudic->b = 0.0;
    prudic->d = 0.0;

    prudic->korm_norm = 0.0;
    prudic->price = 0.0;
    prudic->korm_price = 0.0;

    return true;
}



bool Back::add_fish(int idi, int young, int old) {
    Prud* prudic = find_prud(idi);
    /*for (auto& i : state.pruds) {
        if (i.id == idi) {

            break;
        }
    }*/

    if (!prudic)
        return false;

    if (!prudic->active)
        return false;

    if (prudic->cleaning)
        return false;

    if (young < 0 || old < 0)
        return false;

    prudic->fish.young += young;
    prudic->fish.old += old;
    state.totalFish += young + old;

    return true;
}



bool Back::sell_fish(int idi, int sum) {

    /*for (auto& i : state.pruds) {
        if (i.id == idi) {
            break;
        }
    }*/
    Prud* prudic = find_prud(idi);
    if (!prudic)
        return false;

    if (!prudic->active)
        return false;

    if (prudic->cleaning)
        return false;

    if (sum <= 0)
        return false;

    if (sum > prudic->fish.old)
        return false;

    prudic->fish.old -= sum;

    double profit = sum * prudic->price;

    state.money += profit;
    state.totalFish -= sum;

    if (fanction.for_sale) {
        fanction.for_sale(idi, sum, profit);
    }

    about();
    return true;
}



bool Back::buy_korm(int idi, double sum) {
    Prud* prudic = find_prud(idi);
    /*for (auto& i : state.pruds) {
        if (i.id == idi) {
            break;
        }
    }*/
    if (!prudic)
        return false;

    if (!prudic->active)
        return false;

    if (sum <= 0.0)
        return false;

    double cost = sum * prudic->korm_price;

    if (cost > state.money)
        return false;

    state.money -= cost;

    auto x = find_if(state.pruds.begin(), state.pruds.end(), [idi](const Prud& p) {
        return p.id == idi;
        });

    if (x == state.pruds.end())
        return false;

    int  i = distance(state.pruds.begin(), x);

    korm[i] += sum;

    if (fanction.to_korm) {
        fanction.to_korm(idi, sum, cost);
    }

    /*
    double cost = sum  * 50.0;

    if (state.money >= cost)
    {
        state.money -= cost;
        feed[idi] += sum;
    }
    */
    about();

    return true;
}


void Back::nextWeek() {
    if (state.no_money)
        return;

    in_week();
}



void Back::in_week() {
    ++state.week;

    if (fanction.new_week_start)
        fanction.new_week_start(state.week);

    prud_do();

    cleaning_do();

    do_contract();

    check_money_0();

    about();
}



void Back::prud_do()
{
    for (Prud& i : state.pruds)
    {
        if (!i.active)
            continue;

        if (i.cleaning)
            continue;

        in_prud(i);
    }
}



void Back::in_prud(Prud& prud) {
    korm_fish(prud);

    new_fish_do(prud);

    random_loss(prud);
}



void Back::korm_fish(Prud& prud) {
    const auto it = std::find_if(state.pruds.begin(), state.pruds.end(), [&prud](const Prud& p) {
        return p.id == prud.id;
        });

    if (it == state.pruds.end())
        return;

    int i = distance(state.pruds.begin(), it);

    double need = prud.fish.old * prud.korm_norm;


    if (korm[i] >= need)
    {
        korm[i] -= need;
        return;
    }

    double have = korm[i];

    korm[i] = 0.0;

    double ratio = need > 0.0 ? have / need : 1.0;

    prud.fish.old = int(prud.fish.old * ratio);
}



void Back::new_fish_do(Prud& prud) {
    if (prud.fish.old <= 0)
        return;

    const double x = prud.fish.old * prud.a;

    const int new_i = int(x);

    const int plus = int(new_i * prud.b);

    prud.fish.young += plus;

    state.totalFish += plus;
}




void Back::random_loss(Prud& prud) {
    if (prud.fish.old <= 0)
        return;

    double ver = prud.d;

    if (ver < 0.0)
        ver = 0.0;

    if (ver > 1.0)
        ver = 1.0;

    int minus = 0;

    for (int i = 0; i < prud.fish.old; ++i) {
        double x = double(random()) / double(random.max());
        if (x < ver)
            ++minus;
    }

    prud.fish.old -= minus;
    state.totalFish -= minus;
}


bool Back::start_cleaning(int idi)
{
    Prud* prudic = find_prud(idi);

    if (!prudic)
        return false;

    if (!prudic->active)
        return false;

    if (prudic->cleaning)
        return false;

    prudic->cleaning = true;

    not_active_prud.push_back(idi);

    if (fanction.clean_start)
        fanction.clean_start(idi);

    return true;
}


void Back::cleaning_do() {
    vector<int> curr = not_active_prud;

    not_active_prud.clear();

    for (int i : curr) {
        Prud* prudic = find_prud(i);

        if (!prudic)
            continue;

        prudic->cleaning = false;

        if (fanction.clean_finish)
            fanction.clean_finish(i);
    }
}



void Back::createContract(int weeks, double week_korm, int need_fish, double minus_kg)
{
    if (weeks <= 0)
        return;

    state.contracts_svo.active = true;

    state.contracts_svo.all_weeks = weeks;
    state.contracts_svo.curr = 0;

    state.contracts_svo.week_korm = week_korm;

    state.contracts_svo.need_fish = need_fish;

    state.contracts_svo.minus_kg = minus_kg;
}


void Back::delete_contract() {
    state.contracts_svo = contract{};
}


void Back::do_contract() {
    if (!state.contracts_svo.active)
        return;

    ++state.contracts_svo.curr;

    state.money -= state.contracts_svo.week_korm;

    if (state.contracts_svo.curr >= state.contracts_svo.all_weeks) {
        int poter = max(0, state.contracts_svo.need_fish - state.totalFish);

        state.money -= poter * state.contracts_svo.minus_kg;

        state.contracts_svo.active = false;

        if (fanction.contract_finish)
            fanction.contract_finish();
    }
}


void Back::check_money_0() {
    if (state.money >= 0.0)
        return;

    state.no_money = true;

    if (fanction.money_0)
        fanction.money_0();
}



const Back::State&
Back::get_state() const {
    return state;
}


double Back::get_money() const {
    return state.money;
}


int Back::get_all_fish() const {
    return state.totalFish;
}


bool Back::not_money() const {
    return state.no_money;
}

void Back::about() {
    if (fanction.Changed)
        fanction.Changed(state);
}
