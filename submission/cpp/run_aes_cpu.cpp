#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <algorithm>
#include "heterosta3d.v1.0.20260107/include/heterosta3d.h"

int main() {
    const char* lic_2d = "lic:heterosta:eyJhbGciOiJFZERTQSJ9.eyJlbWFpbCI6ImRpbmdfamlhbGlhbmdAZ210Y24uY29tIiwiaW5zdGl0dXRpb24iOiJOVklESUEiLCJmaXJzdF9uYW1lIjoiZGluZyIsImxhc3RfbmFtZSI6ImhzZ2giLCJjb21tZXJjaWFsIjpmYWxzZSwiaGFyZHdhcmVfaWQiOiIiLCJleHAiOjE3ODgwMTk5MTYsImlhdCI6MTc4MDA3MTExNn0.YchEYrA_So7Dw114VGnNkxnvQxRr3jBO6wjNGV0IwBMbnvHODB9sxa4mwWPo0W-bFKRaDlRN84JNfE7ZxMuWCA";
    const char* lic_3d = "lic:heterosta3d:eyJhbGciOiJFZERTQSJ9.eyJlbWFpbCI6ImRpbmdfamlhbGlhbmdAZ210Y24uY29tIiwiaW5zdGl0dXRpb24iOiJOVklESUEiLCJmaXJzdF9uYW1lIjoiZGluZyIsImxhc3RfbmFtZSI6ImhzZ2giLCJjb21tZXJjaWFsIjpmYWxzZSwiaGFyZHdhcmVfaWQiOiIiLCJleHAiOjE3ODc5MjE1MzYsImlhdCI6MTc3OTk3MjczNn0.-7jvgNMiql7XGYrukhAGyA-jkmQc6QMGH6K-Kt2ny6vTtSX4x7nPV2uTyYzFZzfT3534BNG9OunlBdo7B9CZDw";

    if (!heterosta3d_init_license(lic_2d, lic_3d)) {
        std::cerr << "[FATAL ERROR] Failed to initialize licenses." << std::endl;
        return 1;
    }

    Heterosta3D *sta = heterosta3d_new();
    if (!sta) {
        std::printf("Failed to create Heterosta3D\n");
        return 1;
    }

    const char *base = "/mnt/f/reasonix/OpenROAD-flow-scripts/flow/platforms/asap7/lib/";

    std::string ss_e_invbuf = std::string(base) + "asap7sc7p5t_INVBUF_RVT_SS_nldm_201020.lib";
    std::string ss_e_simple = std::string(base) + "asap7sc7p5t_SIMPLE_RVT_SS_nldm_201020.lib";
    std::string ss_e_ao     = std::string(base) + "asap7sc7p5t_AO_RVT_SS_nldm_201020.lib";
    std::string ss_e_oa     = std::string(base) + "asap7sc7p5t_OA_RVT_SS_nldm_201020.lib";
    std::string ss_e_seq    = std::string(base) + "asap7sc7p5t_SEQ_RVT_SS_nldm_201020.lib";
    const char *ss_early[]  = {ss_e_invbuf.c_str(), ss_e_simple.c_str(), ss_e_ao.c_str(), ss_e_oa.c_str(), ss_e_seq.c_str()};

    std::string ss_l_invbuf = std::string(base) + "asap7sc7p5t_INVBUF_RVT_SS_nldm_201020.lib";
    std::string ss_l_simple = std::string(base) + "asap7sc7p5t_SIMPLE_RVT_SS_nldm_201020.lib";
    std::string ss_l_ao     = std::string(base) + "asap7sc7p5t_AO_RVT_SS_nldm_201020.lib";
    std::string ss_l_oa     = std::string(base) + "asap7sc7p5t_OA_RVT_SS_nldm_201020.lib";
    std::string ss_l_seq    = std::string(base) + "asap7sc7p5t_SEQ_RVT_SS_nldm_201020.lib";
    const char *ss_late[]   = {ss_l_invbuf.c_str(), ss_l_simple.c_str(), ss_l_ao.c_str(), ss_l_oa.c_str(), ss_l_seq.c_str()};

    std::string ff_e_invbuf = std::string(base) + "asap7sc7p5t_INVBUF_RVT_FF_nldm_201020.lib";
    std::string ff_e_simple = std::string(base) + "asap7sc7p5t_SIMPLE_RVT_FF_nldm_201020.lib";
    std::string ff_e_ao     = std::string(base) + "asap7sc7p5t_AO_RVT_FF_nldm_201020.lib";
    std::string ff_e_oa     = std::string(base) + "asap7sc7p5t_OA_RVT_FF_nldm_201020.lib";
    std::string ff_e_seq    = std::string(base) + "asap7sc7p5t_SEQ_RVT_FF_nldm_201020.lib";
    const char *ff_early[]  = {ff_e_invbuf.c_str(), ff_e_simple.c_str(), ff_e_ao.c_str(), ff_e_oa.c_str(), ff_e_seq.c_str()};

    std::string ff_l_invbuf = std::string(base) + "asap7sc7p5t_INVBUF_RVT_FF_nldm_201020.lib";
    std::string ff_l_simple = std::string(base) + "asap7sc7p5t_SIMPLE_RVT_FF_nldm_201020.lib";
    std::string ff_l_ao     = std::string(base) + "asap7sc7p5t_AO_RVT_FF_nldm_201020.lib";
    std::string ff_l_oa     = std::string(base) + "asap7sc7p5t_OA_RVT_FF_nldm_201020.lib";
    std::string ff_l_seq    = std::string(base) + "asap7sc7p5t_SEQ_RVT_FF_nldm_201020.lib";
    const char *ff_late[]   = {ff_l_invbuf.c_str(), ff_l_simple.c_str(), ff_l_ao.c_str(), ff_l_oa.c_str(), ff_l_seq.c_str()};

    bool ok = true;

    ok &= heterosta3d_create_liberty_set_batch(sta, EL_EARLY, "top_ss", ss_early, 5);
    ok &= heterosta3d_create_liberty_set_batch(sta, EL_LATE,  "top_ss", ss_late, 5);

    ok &= heterosta3d_create_liberty_set_batch(sta, EL_EARLY, "top_ff", ff_early, 5);
    ok &= heterosta3d_create_liberty_set_batch(sta, EL_LATE,  "top_ff", ff_late, 5);

    ok &= heterosta3d_create_liberty_set_batch(sta, EL_EARLY, "btm_ss", ss_early, 5);
    ok &= heterosta3d_create_liberty_set_batch(sta, EL_LATE,  "btm_ss", ss_late, 5);

    ok &= heterosta3d_create_liberty_set_batch(sta, EL_EARLY, "btm_ff", ff_early, 5);
    ok &= heterosta3d_create_liberty_set_batch(sta, EL_LATE,  "btm_ff", ff_late, 5);

    const char *corner_names[] = {"ss_ss", "ss_ff", "ff_ss", "ff_ff"};
    const char *top_sets[] = {"top_ss", "top_ss", "top_ff", "top_ff"};
    const char *btm_sets[] = {"btm_ss", "btm_ff", "btm_ss", "btm_ff"};

    for (int i = 0; i < 4; ++i) {
        ok &= heterosta3d_create_delay_corner(sta, corner_names[i], top_sets[i], btm_sets[i], HETEROSTA3D_CPU_DEVICE_ID);
    }
    if (!ok) {
        std::printf("create delay corner failed\n");
        heterosta3d_free(sta);
        return 1;
    }

    ok &= heterosta3d_read_netlist(sta, "3d_partitioned/aes/aes_heterosta3d.v");
    if (!ok) {
        std::printf("read netlist failed\n");
        heterosta3d_free(sta);
        return 1;
    }

    size_t num_pins = 50000;
    size_t num_nets = 70000;

    heterosta3d_flatten_all(sta);
    heterosta3d_build_graph(sta);

    for (int i = 0; i < 4; ++i) {
        heterosta3d_read_sdc(sta, "constraints/aes_complete.sdc", corner_names[i]);
    }

    std::vector<float> pos_x(num_pins);
    std::vector<float> pos_y(num_pins);
    std::vector<float> hbt_x(num_nets);
    std::vector<float> hbt_y(num_nets);
    for (size_t i = 0; i < num_pins; ++i) {
        pos_x[i] = 100.0f + (float)(i % 400) * 2.0f;
        pos_y[i] = (i < num_pins/2) ? 100.0f + (float)(i / 400) * 3.0f
                                     : -100.0f - (float)((i - num_pins/2) / 400) * 3.0f;
    }
    for (size_t i = 0; i < num_nets; ++i) {
        hbt_x[i] = 200.0f;
        hbt_y[i] = 0.0f;
    }

    float unit_cap_x_top = 0.002f, unit_cap_y_top = 0.002f;
    float unit_res_x_top = 0.0005f, unit_res_y_top = 0.0005f;
    float unit_cap_x_btm = 0.002f, unit_cap_y_btm = 0.002f;
    float unit_res_x_btm = 0.0005f, unit_res_y_btm = 0.0005f;
    float hbt_r = 0.003f, hbt_c = 0.6f;

    for (int i = 0; i < 4; ++i) {
        heterosta3d_extract_rc_from_placement(
            sta, pos_x.data(), pos_y.data(), hbt_x.data(), hbt_y.data(),
            unit_cap_x_top, unit_cap_y_top, unit_res_x_top, unit_res_y_top,
            unit_cap_x_btm, unit_cap_y_btm, unit_res_x_btm, unit_res_y_btm,
            hbt_r, hbt_c, 4, corner_names[i]);
    }

    for (int i = 0; i < 4; ++i) {
        heterosta3d_update_delay(sta, corner_names[i]);
        heterosta3d_update_arrivals(sta, corner_names[i]);
    }

    // ═══ 4-Corner WNS/TNS Report ═══
    std::printf("=== AES 4-Corner Results (CPU) ===\n");
    std::printf("%-8s %-16s %-16s %-16s %-16s\n",
                "Corner", "Setup WNS(ps)", "Setup TNS(ps)", "Hold WNS(ps)", "Hold TNS(ps)");
    std::printf("------------------------------------------------------------------\n");

    std::vector<float> slack(num_pins * 2);
    float (*slack_ptr)[2] = reinterpret_cast<float (*)[2]>(slack.data());

    for (int i = 0; i < 4; ++i) {
        float wns_max, tns_max, wns_min, tns_min;
        heterosta3d_report_wns_tns_max(sta, &wns_max, &tns_max, corner_names[i]);
        heterosta3d_report_wns_tns_min(sta, &wns_min, &tns_min, corner_names[i]);

        std::printf("%-8s %-16.4f %-16.4f %-16.4f %-16.4f\n",
                    corner_names[i], wns_max, tns_max, wns_min, tns_min);

        char paths_max_file[64], paths_min_file[64];
        std::sprintf(paths_max_file, "aes_paths_max_%s.rpt", corner_names[i]);
        std::sprintf(paths_min_file, "aes_paths_min_%s.rpt", corner_names[i]);
        heterosta3d_dump_paths_max_to_file(sta, 10, 1, paths_max_file, corner_names[i]);
        heterosta3d_dump_paths_min_to_file(sta, 10, 1, paths_min_file, corner_names[i]);
    }

    // ═══ HBT Capacitance Sweep ═══
    std::printf("\n=== AES HBT Capacitance Sweep (ss_ff corner) ===\n");
    std::printf("%-10s %-16s %-16s\n", "C_hbt(fF)", "Setup WNS(ps)", "Hold WNS(ps)");
    std::printf("----------------------------------------------\n");

    FILE *csv = std::fopen("aes_hbt_sweep.csv", "w");
    std::fprintf(csv, "C_hbt_fF,Setup_WNS_ps,Hold_WNS_ps\n");

    heterosta3d_free(sta);

    for (float sweep_c = 0.1f; sweep_c <= 5.1f; sweep_c += 0.5f) {
        sta = heterosta3d_new();
        ok = true;
        ok &= heterosta3d_create_liberty_set_batch(sta, EL_EARLY, "top_ss", ss_early, 5);
        ok &= heterosta3d_create_liberty_set_batch(sta, EL_LATE,  "top_ss", ss_late, 5);
        ok &= heterosta3d_create_liberty_set_batch(sta, EL_EARLY, "btm_ff", ff_early, 5);
        ok &= heterosta3d_create_liberty_set_batch(sta, EL_LATE,  "btm_ff", ff_late, 5);
        heterosta3d_create_delay_corner(sta, "ss_ff", "top_ss", "btm_ff", HETEROSTA3D_CPU_DEVICE_ID);
        heterosta3d_read_netlist(sta, "3d_partitioned/aes/aes_heterosta3d.v");
        heterosta3d_flatten_all(sta);
        heterosta3d_build_graph(sta);
        heterosta3d_read_sdc(sta, "constraints/aes_complete.sdc", "ss_ff");

        heterosta3d_extract_rc_from_placement(
            sta, pos_x.data(), pos_y.data(), hbt_x.data(), hbt_y.data(),
            unit_cap_x_top, unit_cap_y_top, unit_res_x_top, unit_res_y_top,
            unit_cap_x_btm, unit_cap_y_btm, unit_res_x_btm, unit_res_y_btm,
            hbt_r, sweep_c, 4, "ss_ff");

        heterosta3d_update_delay(sta, "ss_ff");
        heterosta3d_update_arrivals(sta, "ss_ff");

        float setup_wns, setup_tns, hold_wns, hold_tns;
        heterosta3d_report_wns_tns_max(sta, &setup_wns, &setup_tns, "ss_ff");
        heterosta3d_report_wns_tns_min(sta, &hold_wns, &hold_tns, "ss_ff");

        std::printf("%-10.1f %-16.4f %-16.4f\n", sweep_c, setup_wns, hold_wns);
        std::fprintf(csv, "%.1f,%.4f,%.4f\n", sweep_c, setup_wns, hold_wns);

        heterosta3d_free(sta);
    }
    std::fclose(csv);
    std::printf("=== Sweep Complete → aes_hbt_sweep.csv ===\n");
    return 0;
}
