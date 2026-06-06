#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <string>
#include <limits> 


const double R_AIR = 287.0;  // J/(kg*K)
const double CV_AIR = 718.0; // J/(kg*K)
const double CP_AIR = R_AIR + CV_AIR; // Cp = R + Cv
const double GAMMA_AIR = CP_AIR / CV_AIR; // ~1.4


struct State {
    double p = 0, v = 0, t = 0, s = 0;
};

struct CycleResults {
    State state1, state2, state3, state4;
    double work_net = 0, efficiency = 0;
    std::string cycle_name;
    std::vector<double> v_12, p_12, v_23, p_23, v_34, p_34, v_41, p_41;
    std::vector<double> s_12, t_12, s_23, t_23, s_34, t_34, s_41, t_41;
};


CycleResults simulate_otto_cycle(double p1, double t1, double cr);
CycleResults simulate_carnot_cycle(double t_high, double t_low, double p1, double v1, double v2);
CycleResults simulate_diesel_cycle(double p1, double t1, double cr, double rc);
CycleResults simulate_brayton_cycle(double p1, double t1, double pr, double t3_max);
void save_data_to_csv(const CycleResults& results);


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./thermodynamic_simulator [cycle_name] [params...]" << std::endl;
        return 1;
    }

    std::string cycle_name = argv[1];
    CycleResults results;

    try {
        if (cycle_name == "otto") {
            double p1 = std::stod(argv[2]);
            double t1 = std::stod(argv[3]);
            double cr = std::stod(argv[4]);
            results = simulate_otto_cycle(p1, t1, cr);
        }
        else if (cycle_name == "carnot") {
            double t_high = std::stod(argv[2]);
            double t_low = std::stod(argv[3]);
            double p1 = std::stod(argv[4]);
            double v1 = std::stod(argv[5]);
            double v2 = std::stod(argv[6]);
            results = simulate_carnot_cycle(t_high, t_low, p1, v1, v2);
        }
        else if (cycle_name == "diesel") {
            double p1 = std::stod(argv[2]);
            double t1 = std::stod(argv[3]);
            double cr = std::stod(argv[4]);
            double rc = std::stod(argv[5]);
            results = simulate_diesel_cycle(p1, t1, cr, rc);
        }
        else if (cycle_name == "brayton") {
            double p1 = std::stod(argv[2]);
            double t1 = std::stod(argv[3]);
            double pr = std::stod(argv[4]);
            double t3_max = std::stod(argv[5]);
            results = simulate_brayton_cycle(p1, t1, pr, t3_max);
        }
        else {
            std::cerr << "Invalid cycle name: " << cycle_name << std::endl;
            return 1;
        }

        save_data_to_csv(results);
        
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Work_Net: " << results.work_net / 1000 << " kJ/kg" << std::endl;
        std::cout << "Efficiency: " << results.efficiency << "%" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing arguments: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}



CycleResults simulate_otto_cycle(double p1, double t1, double cr) {
    CycleResults res; res.cycle_name = "Otto";
    
    // Calculate V1 first
    double v1 = (R_AIR * t1) / p1;
    // Set arbitrary initial entropy S = P * V^gamma
    double s1 = p1 * pow(v1, GAMMA_AIR); 

    res.state1 = {p1, v1, t1, s1};
    res.state2.v = res.state1.v / cr; res.state2.p = p1 * pow(cr, GAMMA_AIR);
    res.state2.t = t1 * pow(cr, GAMMA_AIR - 1); res.state2.s = res.state1.s;
    double t3_max = res.state2.t * 3.0; double q_in = CV_AIR * (t3_max - res.state2.t); 
    res.state3.t = t3_max; res.state3.v = res.state2.v;
    res.state3.p = res.state2.p * (res.state3.t / res.state2.t); 
    res.state3.s = res.state2.s + CV_AIR * log(res.state3.t / res.state2.t);
    res.state4.v = res.state1.v; res.state4.t = res.state3.t * pow(1 / cr, GAMMA_AIR - 1);
    res.state4.p = res.state3.p * pow(1 / cr, GAMMA_AIR); res.state4.s = res.state3.s;
    double q_out = CV_AIR * (res.state4.t - res.state1.t); res.work_net = q_in - q_out;
    res.efficiency = (res.work_net / q_in) * 100;
    
    int n_points = 100;
    for(int i=0; i<n_points; ++i) { double v = res.state1.v + i * (res.state2.v - res.state1.v) / (n_points - 1);
        res.v_12.push_back(v); res.p_12.push_back(res.state1.p * pow(res.state1.v / v, GAMMA_AIR));
        res.s_12.push_back(res.state1.s); res.t_12.push_back(res.state1.t * pow(res.state1.v / v, GAMMA_AIR - 1)); }
    for(int i=0; i<n_points; ++i) { double t = res.state2.t + i * (res.state3.t - res.state2.t) / (n_points - 1);
        res.v_23.push_back(res.state2.v); res.p_23.push_back(res.state2.p * (t / res.state2.t));
        res.s_23.push_back(res.state2.s + CV_AIR * log(t / res.state2.t)); res.t_23.push_back(t); }
    for(int i=0; i<n_points; ++i) { double v = res.state3.v + i * (res.state4.v - res.state3.v) / (n_points - 1);
        res.v_34.push_back(v); res.p_34.push_back(res.state3.p * pow(res.state3.v / v, GAMMA_AIR));
        res.s_34.push_back(res.state3.s); res.t_34.push_back(res.state3.t * pow(res.state3.v / v, GAMMA_AIR - 1)); }
    for(int i=0; i<n_points; ++i) { double t = res.state4.t + i * (res.state1.t - res.state4.t) / (n_points - 1);
        res.v_41.push_back(res.state4.v); res.p_41.push_back(res.state4.p * (t / res.state4.t));
        res.s_41.push_back(res.state4.s + CV_AIR * log(t / res.state4.t)); res.t_41.push_back(t); }
    return res;
}

CycleResults simulate_carnot_cycle(double t_high, double t_low, double p1, double v1, double v2) {
    CycleResults res; res.cycle_name = "Carnot";
    
    // Set arbitrary initial entropy S = P * V^gamma
    double s1 = p1 * pow(v1, GAMMA_AIR);
    
    res.state1 = {p1, v1, t_high, s1}; 
    res.state2 = {p1 * v1 / v2, v2, t_high, s1 + R_AIR * log(v2 / v1)};
    double v3_exp = 1.0 / (GAMMA_AIR - 1.0); res.state3.v = res.state2.v * pow(t_high / t_low, v3_exp);
    res.state3.p = res.state2.p * pow(res.state2.v / res.state3.v, GAMMA_AIR); res.state3.t = t_low; res.state3.s = res.state2.s;
    res.state4.v = res.state1.v * pow(t_high / t_low, v3_exp); res.state4.p = res.state1.p * pow(res.state1.v / res.state4.v, GAMMA_AIR);
    res.state4.t = t_low; res.state4.s = res.state1.s;
    double q_in = R_AIR * t_high * log(res.state2.v / res.state1.v); res.efficiency = (1.0 - (t_low / t_high)) * 100;
    res.work_net = q_in * (res.efficiency / 100.0);
    int n_points = 100;
    for(int i=0; i<n_points; ++i) { double v = res.state1.v + i * (res.state2.v - res.state1.v) / (n_points - 1);
        res.v_12.push_back(v); res.p_12.push_back(res.state1.p * res.state1.v / v);
        res.s_12.push_back(res.state1.s + R_AIR * log(v / res.state1.v)); res.t_12.push_back(t_high); }
    for(int i=0; i<n_points; ++i) { double v = res.state2.v + i * (res.state3.v - res.state2.v) / (n_points - 1);
        res.v_23.push_back(v); res.p_23.push_back(res.state2.p * pow(res.state2.v / v, GAMMA_AIR));
        res.s_23.push_back(res.state2.s); res.t_23.push_back(res.state2.t * pow(res.state2.v / v, GAMMA_AIR - 1)); }
    for(int i=0; i<n_points; ++i) { double v = res.state3.v + i * (res.state4.v - res.state3.v) / (n_points - 1);
        res.v_34.push_back(v); res.p_34.push_back(res.state3.p * res.state3.v / v);
        res.s_34.push_back(res.state3.s + R_AIR * log(v / res.state3.v)); res.t_34.push_back(t_low); }
    for(int i=0; i<n_points; ++i) { double v = res.state4.v + i * (res.state1.v - res.state4.v) / (n_points - 1);
        res.v_41.push_back(v); res.p_41.push_back(res.state4.p * pow(res.state4.v / v, GAMMA_AIR));
        res.s_41.push_back(res.state4.s); res.t_41.push_back(res.state4.t * pow(res.state4.v / v, GAMMA_AIR - 1)); }
    return res;
}

CycleResults simulate_diesel_cycle(double p1, double t1, double cr, double rc) {
    CycleResults res; res.cycle_name = "Diesel";
    
    // Calculate V1 first
    double v1 = (R_AIR * t1) / p1;
    // Set arbitrary initial entropy S = P * V^gamma
    double s1 = p1 * pow(v1, GAMMA_AIR);

    res.state1 = {p1, v1, t1, s1};
    res.state2.v = res.state1.v / cr; res.state2.p = p1 * pow(cr, GAMMA_AIR);
    res.state2.t = t1 * pow(cr, GAMMA_AIR - 1); res.state2.s = res.state1.s;
    res.state3.v = res.state2.v * rc; res.state3.p = res.state2.p;
    res.state3.t = res.state2.t * rc; res.state3.s = res.state2.s + CP_AIR * log(res.state3.t / res.state2.t);
    res.state4.v = res.state1.v; double expansion_ratio = res.state4.v / res.state3.v;
    res.state4.t = res.state3.t * pow(1 / expansion_ratio, GAMMA_AIR - 1);
    res.state4.p = res.state3.p * pow(1 / expansion_ratio, GAMMA_AIR); res.state4.s = res.state3.s;
    double q_in = CP_AIR * (res.state3.t - res.state2.t); double q_out = CV_AIR * (res.state4.t - res.state1.t);
    res.work_net = q_in - q_out; res.efficiency = (res.work_net / q_in) * 100;
    int n_points = 100;
    for(int i=0; i<n_points; ++i) { double v = res.state1.v + i * (res.state2.v - res.state1.v) / (n_points - 1);
        res.v_12.push_back(v); res.p_12.push_back(res.state1.p * pow(res.state1.v / v, GAMMA_AIR));
        res.s_12.push_back(res.state1.s); res.t_12.push_back(res.state1.t * pow(res.state1.v / v, GAMMA_AIR - 1)); }
    for(int i=0; i<n_points; ++i) { double v = res.state2.v + i * (res.state3.v - res.state2.v) / (n_points - 1);
        double t = res.state2.t * (v / res.state2.v); res.v_23.push_back(v); res.p_23.push_back(res.state2.p);
        res.s_23.push_back(res.state2.s + CP_AIR * log(t / res.state2.t)); res.t_23.push_back(t); }
    for(int i=0; i<n_points; ++i) { double v = res.state3.v + i * (res.state4.v - res.state3.v) / (n_points - 1);
        res.v_34.push_back(v); res.p_34.push_back(res.state3.p * pow(res.state3.v / v, GAMMA_AIR));
        res.s_34.push_back(res.state3.s); res.t_34.push_back(res.state3.t * pow(res.state3.v / v, GAMMA_AIR - 1)); }
    for(int i=0; i<n_points; ++i) { double t = res.state4.t + i * (res.state1.t - res.state4.t) / (n_points - 1);
        res.v_41.push_back(res.state4.v); res.p_41.push_back(res.state4.p * (t / res.state4.t));
        res.s_41.push_back(res.state4.s + CV_AIR * log(t / res.state4.t)); res.t_41.push_back(t); }
    return res;
}

CycleResults simulate_brayton_cycle(double p1, double t1, double pr, double t3_max) {
    CycleResults res; res.cycle_name = "Brayton"; double gamma_exp = (GAMMA_AIR - 1.0) / GAMMA_AIR;
    
    // Calculate V1 first
    double v1 = (R_AIR * t1) / p1;
    // Set arbitrary initial entropy S = P * V^gamma
    double s1 = p1 * pow(v1, GAMMA_AIR);

    res.state1 = {p1, v1, t1, s1};
    res.state2.p = p1 * pr; res.state2.t = t1 * pow(pr, gamma_exp);
    res.state2.v = (R_AIR * res.state2.t) / res.state2.p; res.state2.s = res.state1.s;
    res.state3.t = t3_max; res.state3.p = res.state2.p;
    res.state3.v = (R_AIR * res.state3.t) / res.state3.p; res.state3.s = res.state2.s + CP_AIR * log(res.state3.t / res.state2.t);
    res.state4.p = p1; res.state4.t = res.state3.t / pow(pr, gamma_exp);
    res.state4.v = (R_AIR * res.state4.t) / res.state4.p; res.state4.s = res.state3.s;
    double q_in = CP_AIR * (res.state3.t - res.state2.t); double q_out = CP_AIR * (res.state4.t - res.state1.t);
    res.work_net = q_in - q_out; res.efficiency = (res.work_net / q_in) * 100;
    int n_points = 100;
    for(int i=0; i<n_points; ++i) { double p = res.state1.p + i * (res.state2.p - res.state1.p) / (n_points - 1);
        double t = res.state1.t * pow(p / res.state1.p, gamma_exp);
        res.v_12.push_back((R_AIR * t) / p); res.p_12.push_back(p);
        res.s_12.push_back(res.state1.s); res.t_12.push_back(t); }
    for(int i=0; i<n_points; ++i) { double t = res.state2.t + i * (res.state3.t - res.state2.t) / (n_points - 1);
        res.v_23.push_back((R_AIR * t) / res.state2.p); res.p_23.push_back(res.state2.p);
        res.s_23.push_back(res.state2.s + CP_AIR * log(t / res.state2.t)); res.t_23.push_back(t); }
    for(int i=0; i<n_points; ++i) { double p = res.state3.p + i * (res.state4.p - res.state3.p) / (n_points - 1);
        double t = res.state3.t * pow(p / res.state3.p, gamma_exp);
        res.v_34.push_back((R_AIR * t) / p); res.p_34.push_back(p);
        res.s_34.push_back(res.state3.s); res.t_34.push_back(t); }
    for(int i=0; i<n_points; ++i) { double t = res.state4.t + i * (res.state1.t - res.state4.t) / (n_points - 1);
        res.v_41.push_back((R_AIR * t) / res.state4.p); res.p_41.push_back(res.state4.p);
        res.s_41.push_back(res.state4.s + CP_AIR * log(t / res.state4.t)); res.t_41.push_back(t); }
    return res;
}

void save_data_to_csv(const CycleResults& results) {
    auto write_pv_file = [](const std::string& f, const auto& v, const auto& p){
        std::ofstream file(f); file << "v,p\n";
        for (size_t i = 0; i < v.size(); ++i) file << v[i] << "," << p[i] << "\n";
    };
    auto write_ts_file = [](const std::string& f, const auto& s, const auto& t){
        std::ofstream file(f); file << "s,t\n";
        for (size_t i = 0; i < s.size(); ++i) file << s[i] << "," << t[i] << "\n";
    };
    write_pv_file("pv_process12.csv", results.v_12, results.p_12);
    write_pv_file("pv_process23.csv", results.v_23, results.p_23);
    write_pv_file("pv_process34.csv", results.v_34, results.p_34);
    write_pv_file("pv_process41.csv", results.v_41, results.p_41);
    write_ts_file("ts_process12.csv", results.s_12, results.t_12);
    write_ts_file("ts_process23.csv", results.s_23, results.t_23);
    write_ts_file("ts_process34.csv", results.s_34, results.t_34);
    write_ts_file("ts_process41.csv", results.s_41, results.t_41);
    std::ofstream states_file("states.csv");
    states_file << "v,p,t,s,label\n";
    states_file << results.state1.v << "," << results.state1.p << "," << results.state1.t << "," << results.state1.s << ",1\n";
    states_file << results.state2.v << "," << results.state2.p << "," << results.state2.t << "," << results.state2.s << ",2\n";
    states_file << results.state3.v << "," << results.state3.p << "," << results.state3.t << "," << results.state3.s << ",3\n";
    states_file << results.state4.v << "," << results.state4.p << "," << results.state4.t << "," << results.state4.s << ",4\n";
}