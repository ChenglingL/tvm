/* ---------------------------------------------------------------------------------
 * Copyright 2021-2023 Tao Zhang
 *
 * This file is part of TVM.
 *
 * TVM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation,
 * either version 3 of the License,
 * or (at your option) any later version.
 *
 * TVM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TVM. If not, see <https://www.gnu.org/licenses/>.
 *
 * Author: Tao Zhang @ Shanghai Jiao Tong University, zhangtao.scholar@sjtu.edu.cn
 * Coauthor: Jennifer Schwarz @ Syracuse University, jschwarz@physics.syr.edu
 * ---------------------------------------------------------------------------------
 */

#include <cstdio>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <getopt.h>
#include <unordered_map>
#include <random>
#include "Run/Run.h"

using namespace std;

int     InitializeAll(Run *, int numpts, int idx, string confName);
int     LoadConf(string filename, Run *);

int main(int argc, char *argv[]) {
    int numpts = 200;
    double temp = 0.1;
    double s0 = 5.8;
    int idx = 0;
    double kV = 10.;
    int c;
    while((c=getopt(argc,argv,"k:n:g:m:s:r:a:i:v:b:x:y:z:p:t:e:q:c:")) != -1)
        switch(c)
        {
            case 'n': numpts = atoi(optarg); break;
            case 't': temp = atof(optarg); break;
            case 's': s0 = atof(optarg); break;
            case 'k': kV = atof(optarg); break;
            case 'i': idx = atof(optarg); break;
            case '?':
                    if(optopt=='c')
                        std::cerr<<"Option -" << optopt << "requires an argument.\n";
                    else if(isprint(optopt))
                        std::cerr<<"Unknown option '-" << optopt << "'.\n";
                    else
                        std::cerr << "Unknown option character.\n";
                    return 1;
            default:
                       abort();
        };
    Run * run = new Run();
    run->setWorkDir("/home/chengling/Research/Project/Cell/3dVertex/tvmdata/");
    char confName[256];
    char paraSID[256];
    sprintf(confName,"/home/chengling/Research/Project/Cell/3dVertex/tvm/scripts/submissionScript/conf_N%i_s0%.4f_kV%.4f_T%.8f_id%i",numpts,s0,kV,temp,idx);
    sprintf(paraSID,"_N%i_s0%.4f_kV%.4f_T%.8f_id%i",numpts,s0,kV,temp,idx);
    run->setParasID(paraSID);
    cout<<"reading conf: "<<confName<<endl;
    InitializeAll(run, numpts, idx, confName);
    run->updatePolygonVertices();
//    run->dumpConfigurationVtk();

//    for (auto cell : run->cells_) {
//        cell->updateVolume();
//        printf("%f\n", cell->volume_);
//    }
//    run->cells_[200]->updateVolume();
//    printf("%f\n", run->cells_[200]->volume_);
//    run->reconnection_->Lth_ = 0.5;
//    run->reconnection_->I_H(run->edges_[2700], true);
//    run->reconnection_->Lth_ = 2.0;
//    run->reconnection_->H_I(run->polygons_[run->polygons_.size()-1], true);

    run->start();

    return 0;
}

int InitializeAll(Run * run, int numpts, int idx, string confName) {
    printf("Initialization start ...\n");

    // load initial configuration
    char topoName[256];
    sprintf(topoName,"/home/chengling/Research/Project/Cell/3dVertex/tvm/initialTopo/sample_N%i_id%i.topo",numpts,idx);

    ifstream topofile(topoName);
    if (!topofile.is_open()) {
        cout << "Error opening sample topo file" << endl;
        exit(1);
    }

    string buffer;
    string delimiter = " ";
    size_t pos = 0;
    long int tmp_id;
    vector<string> tokens;
    vector<vector<string>> lines;

    while (getline(topofile, buffer))
    {
        pos = buffer.find((char)13);
        if (pos != string::npos) {
            buffer = buffer.substr(0, pos);
        }
        if (buffer.length() == 0) continue;

        tokens.clear();
        while ((pos = buffer.find(delimiter)) != string::npos) {
            string token = buffer.substr(0, pos);
            if (token.length() > 0) {
                tokens.push_back(token);
            }
            buffer.erase(0, pos + delimiter.length());
        }
        if (buffer.length() > 0) {
            tokens.push_back(buffer);
        }
        lines.push_back(tokens);
    }

    bool verticesFlag = false;
    bool edgesFlag = false;
    bool polygonsFlag = false;
    bool cellsFlag = false;
    bool emptyCellsFlag = false;
    for (int i = 0; i < lines.size(); i++) {
        tokens = lines[i];
        if (tokens[0] == "vertices") {
            verticesFlag = true;
        } else if (tokens[0] == "edges") {
            verticesFlag = false;
            edgesFlag = true;
        } else if (tokens[0] == "polygons") {
            edgesFlag = false;
            polygonsFlag = true;
        } else if (tokens[0] == "cells") {
            polygonsFlag = false;
            cellsFlag = true;
        } else if (tokens[0] == "virtual" && tokens[1] == "cells") {
            cellsFlag = false;
            emptyCellsFlag = true;
        } else {
            if (verticesFlag) {
                tmp_id = atol(tokens[0].c_str());
                Vertex * vertex = new Vertex(run, tmp_id);
                for (int j = 1; j < tokens.size(); j++) {
                    vertex->position_[j - 1] = atof(tokens[j].c_str());
                }
                run->vertices_.push_back(vertex);
            }
            if (edgesFlag) {
                tmp_id = atol(tokens[0].c_str());
                Edge * edge = new Edge(run, tmp_id);
                for (int j = 1; j < tokens.size(); j++) {
                    tmp_id = atol(tokens[j].c_str());
                    for (auto vertex : run->vertices_) {
                        if (vertex->id_ == tmp_id) {
                            edge->vertices_.push_back(vertex);
                            break;
                        }
                    }
                }
                run->edges_.push_back(edge);
            }
            if (polygonsFlag) {
                tmp_id = atol(tokens[0].c_str());
                Polygon * polygon = new Polygon(run, tmp_id);
                for (int j = 1; j < tokens.size(); j++) {
                    tmp_id = atol(tokens[j].c_str());
                    for (auto edge : run->edges_) {
                        if (edge->id_ == tmp_id) {
                            polygon->edges_.push_back(edge);
                            break;
                        }
                    }
                }
                run->polygons_.push_back(polygon);
            }
            if (cellsFlag) {
                tmp_id = atol(tokens[0].c_str());
                Cell * cell = new Cell(run, tmp_id);
                for (int j = 1; j < tokens.size(); j++) {
                    tmp_id = atol(tokens[j].c_str());
                    for (auto polygon : run->polygons_) {
                        if (polygon->id_ == tmp_id) {
                            cell->polygons_.push_back(polygon);
                            break;
                        }
                    }
                }
                run->cells_.push_back(cell);
            }
        }
    }

    for (auto vertex : run->vertices_) {
        if (run->count_vertices_ < vertex->id_ + 1) {
            run->count_vertices_ = vertex->id_ + 1;
        }
    }
    for (auto edge : run->edges_) {
        if (run->count_edges_ < edge->id_ + 1) {
            run->count_edges_ = edge->id_ + 1;
        }
    }
    for (auto polygon : run->polygons_) {
        if (run->count_polygons_ < polygon->id_ + 1) {
            run->count_polygons_ = polygon->id_ + 1;
        }
    }
    for (auto cell : run->cells_) {
        if (run->count_cells_ < cell->id_ + 1) {
            run->count_cells_ = cell->id_ + 1;
        }
    }
    cout << "Number of vertices: " << run->vertices_.size() << endl;
    cout << "Maximum vertex ID: " << run->count_vertices_ - 1 << endl;
    cout << "Number of edges: " << run->edges_.size() << endl;
    cout << "Maximum edge ID: " << run->count_edges_ - 1 << endl;
    cout << "Number of polygons: " << run->polygons_.size() << endl;
    cout << "Maximum polygon ID: " << run->count_polygons_ - 1 << endl;
    cout << "Number of cells: " << run->cells_.size() << endl;
    cout << "Maximum cell ID: " << run->count_cells_ - 1 << endl;

    run->updatePolygonCells();
    long int nPolygon2N = 0;
    for (auto polygon : run->polygons_) {
        if (polygon->cells_.size() == 2) {
            nPolygon2N++;
        }
    }
    cout << "Number of polygons with two neighboring cells: " << nPolygon2N << "/" << run->polygons_.size() << endl;

    // initialize volume object
    run->volume_ = new Volume(run);
    // initialize interface object
    run->interface_ = new Interface(run);
    // initialize reconnection object
    run->reconnection_ = new Reconnection(run);

    LoadConf(confName, run);

    // update geometry and topology information
    run->updateGeoinfo();
    run->updateVertexCells();
    run->volume_->updatePolygonDirections();

    return 0;
}

int LoadConf(string filename, Run * run) {
    ifstream conf(filename.c_str());

    if (!conf.is_open()) {
        cout << "Error opening conf file" << endl;
        exit(1);
    }

    string buffer;
    string delimiter = " ";
    size_t pos = 0;
    vector<string> tokens;
    vector<vector<string>> lines;

    int time_written = 0;
    int dump_written = 0;
    int log_screen_written = 0;
    int s0_written = 0;
    int Lth_written = 0;
    int temperature_written = 0;
    int kv_written = 0;
    int box_written = 0;

    while (getline(conf, buffer))
    {
        pos = buffer.find((char)13);
        if (pos != string::npos) {
            buffer = buffer.substr(0, pos);
        }
        if ((buffer.length() == 0) || (buffer[0] == '#')) continue;

        tokens.clear();
        while ((pos = buffer.find(delimiter)) != string::npos) {
            string token = buffer.substr(0, pos);
            if (token.length() > 0) {
                tokens.push_back(token);
            }
            buffer.erase(0, pos + delimiter.length());
        }
        if (buffer.length() > 0) {
            tokens.push_back(buffer);
        }
        lines.push_back(tokens);
    }

    for (int i = 0; i < lines.size(); i++) {
        tokens = lines[i];
        if (tokens[0] == "time") {
            if (tokens.size() != 4) {
                cerr << "conf file error: ";
                for (int j = 0; j < tokens.size(); j++) {
                    cerr << tokens[j] << " ";
                }
                cerr << endl;
                exit(1);
            }
            run->t_start_ = atof(tokens[1].c_str());
            run->t_end_ = atof(tokens[2].c_str());
            run->dt_ = atof(tokens[3].c_str());
            run->dtr_ = 10*run->dt_;
            time_written = 1;
            cout << "time: " << run->t_start_ << " ~ " << run->t_end_ << " ~ " << run->dt_ << " ~ " << run->dtr_ << endl;
        }
        else if (tokens[0] == "dump") {
            if (tokens.size() != 3) {
                cerr << "conf file error: ";
                for (int j = 0; j < tokens.size(); j++) {
                    cerr << tokens[j] << " ";
                }
                cerr << endl;
                exit(1);
            }
            if (tokens[1] == "vtk") {
                run->dump_period_ = atof(tokens[2].c_str());
                dump_written = 1;
                cout << "dump: " << tokens[1] << " " << run->dump_period_ << endl;
            }
        }
        else if (tokens[0] == "log") {
            if (tokens.size() != 2) {
                cerr << "conf file error: ";
                for (int j = 0; j < tokens.size(); j++) {
                    cerr << tokens[j] << " ";
                }
                cerr << endl;
                exit(1);
            }
            run->log_period_ = atof(tokens[1].c_str());
            log_screen_written = 1;
            cout << "log: " << run->log_period_ << endl;
        }
        else if (tokens[0] == "s0") {
            if (tokens.size() != 2) {
                cerr << "conf file error: ";
                for (int j = 0; j < tokens.size(); j++) {
                    cerr << tokens[j] << " ";
                }
                cerr << endl;
                exit(1);
            }
            run->interface_->s0_ = atof(tokens[1].c_str());
            s0_written = 1;
            cout << "s0: " << run->interface_->s0_ << endl;
        }
        else if (tokens[0] == "Lth") {
            if (tokens.size() != 2 && tokens.size() != 3) {
                cerr << "conf file error: ";
                for (int j = 0; j < tokens.size(); j++) {
                    cerr << tokens[j] << " ";
                }
                cerr << endl;
                exit(1);
            }
            run->reconnection_->Lth_ = atof(tokens[1].c_str());
            if (tokens.size() == 3) {
                if (tokens[2] == "verbose") {
                    run->reconnection_->verbose_ = true;
                } else {
                    cerr << "conf file error: ";
                    for (int j = 0; j < tokens.size(); j++) {
                        cerr << tokens[j] << " ";
                    }
                    cerr << endl;
                    exit(1);
                }
            }
            Lth_written = 1;
            cout << "Lth: " << run->reconnection_->Lth_ << " verbose: " << run->reconnection_->verbose_ << endl;
        }
        else if (tokens[0] == "T") {
            if (tokens.size() != 2) {
                cerr << "conf file error: ";
                for (int j = 0; j < tokens.size(); j++) {
                    cerr << tokens[j] << " ";
                }
                cerr << endl;
                exit(1);
            }
            run->temperature_ = atof(tokens[1].c_str());
            temperature_written = 1;
            cout << "temperature: " << run->temperature_ << endl;
        }
        else if (tokens[0] == "kv") {
            if (tokens.size() != 2) {
                cerr << "conf file error: ";
                for (int j = 0; j < tokens.size(); j++) {
                    cerr << tokens[j] << " ";
                }
                cerr << endl;
                exit(1);
            }
            run->volume_->kv_ = atof(tokens[1].c_str());
            kv_written = 1;
            cout << "kv: " << run->volume_->kv_ << endl;
        }
        else if (tokens[0] == "box") {
            if (tokens.size() != 7) {
                cerr << "conf file error: ";
                for (int j = 0; j < tokens.size(); j++) {
                    cerr << tokens[j] << " ";
                }
                cerr << endl;
                exit(1);
            }
            run->box_ = new Box(run);
            for (int k = 0; k < 3; k++) {
                run->box_->size_[k] = atof(tokens[k + 1].c_str());
                if (run->box_->size_[k] < (-1.0e-6)) {
                    cerr << "conf file error: ";
                    for (int j = 0; j < tokens.size(); j++) {
                        cerr << tokens[j] << " ";
                    }
                    cerr << endl;
                    exit(1);
                }
                if (tokens[k + 4] == "p") {
                    run->box_->boundaryCondition_[k] = true;
                } else if (tokens[k + 4] == "f"){
                    run->box_->boundaryCondition_[k] = false;
                } else {
                    cerr << "conf file error: ";
                    for (int j = 0; j < tokens.size(); j++) {
                        cerr << tokens[j] << " ";
                    }
                    cerr << endl;
                    exit(1);
                }
            }
            box_written = 1;
            cout << "box: " << run->box_->size_[0] << " " << run->box_->size_[1] << " " << run->box_->size_[2] << " ";
            cout << "periodic boundary condition: " << run->box_->boundaryCondition_[0] << " " << run->box_->boundaryCondition_[1] << " " << run->box_->boundaryCondition_[2] << endl;
        }
        else if (tokens[0] == "offsets") {
            if (tokens.size() != 4) {
                cerr << "conf file error: ";
                for (int j = 0; j < tokens.size(); j++) {
                    cerr << tokens[j] << " ";
                }
                cerr << endl;
                exit(1);
            }
            run->n_offsets_= atoi(tokens[1].c_str());
            run->spacing_offsets_= atoi(tokens[2].c_str());
            run->log_spacing_= atof(tokens[3].c_str());
            cout << "number of offsets " << run->n_offsets_ << endl;
            cout << "saving offsets start from 0 with the spacing of " << run->spacing_offsets_ <<endl;
            cout << "log spacing in exponent " << run->log_spacing_ << endl;
        }
        else {
            cerr << "conf file error: ";
            for (int j = 0; j < tokens.size(); j++) {
                cerr << tokens[j] << " ";
            }
            cerr << endl;
            exit(1);
        }
    }

    if (conf.bad() || !conf.eof()) {
        cout << "Error reading file [" << filename << "]" << endl;
        exit(1);
    }

    if (time_written == 0) {
        cout << "conf file error: missing time" << endl;
        exit(1);
    }

    if (dump_written == 0) {
        cout << "conf file error: missing dump" << endl;
        exit(1);
    }

    if (log_screen_written == 0) {
        cout << "conf file error: missing log screen" << endl;
        exit(1);
    }

    if (s0_written == 0) {
        cout << "conf file error: s0" << endl;
        exit(1);
    }

    if (Lth_written == 0) {
        cout << "conf file error: Lth" << endl;
        exit(1);
    }

    if (temperature_written == 0) {
        cout << "conf file error: temperature" << endl;
        exit(1);
    }

    if (kv_written == 0) {
        cout << "conf file error: kv" << endl;
        exit(1);
    }

    if (box_written == 0) {
        cout << "conf file error: box" << endl;
        exit(1);
    }

    conf.close();

    return 0;
}
