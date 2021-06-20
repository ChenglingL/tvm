// Copyright June 2021 Tao Zhang @ Shanghai Jiao Tong University.  All Rights Reserved.
// Author: Tao Zhang @ Shanghai Jiao Tong University, zhangtao.scholar@sjtu.edu.cn
// Corresponding author: Jennifer Schwarz @ Syracuse University, jschwarz@physics.syr.edu

#include <cstdio>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <unordered_map>
#include "Run.h"

using namespace std;

Run::Run() {
    dt_ = 1.0e-4;
    dtr_ = 1.0e-3;
    eta_ = 1.0;
    Lx_ = 13.2;
    Ly_ = 200.0/13.2;
    NCell_ = 400;
    Aic_ = 0.5;
    rho_growth_ = 0.75;
    t_start_ = 0.;
    t_end_ = 4000.;
    dump_period_ = 1.;
    log_period_ = 0.01;
}

int Run::start() {
    count_reconnect_ = 0;
    count_dump_ = 0;
    count_log_ = 0;
    simulation_time_ = t_start_;
    double t_roundError = 0.01*dt_;
    auto start = chrono::steady_clock::now();

    printf("\nSimulation Start ...\n");
    printf("Real time elapsed: Rte\n");
    printf("Time        ");
    printf("Rte         ");
    printf("Volume      ");
    printf("I->H        ");
    printf("H->I        ");
    printf("E_volume    ");
    printf("E_interface ");
    printf("Energy      \n");

    while (simulation_time_ < t_end_ + t_roundError) {
        // update geometry information
        updateGeoinfo();
        // update volumeForces
        volume_->updateForces();
        // update interfaceForces
        interface_->updateForces();
        // update velocities
        updateVerticesVelocity();

        // log to screen
        if (simulation_time_ - t_start_ + t_roundError  > count_log_ * log_period_) {
            volume_->updateEnergy();
            interface_->updateEnergy();
            printf("%-12.2f%-12.3f%-12.3f%-12ld%-12ld%-12.6f%-12.6f%-12.6f\n", simulation_time_,
                   (chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - start).count())/1.0e6,
                   volume_->totalVolume_,
                   reconnection_->count_IH_,
                   reconnection_->count_HI_,
                   volume_->energy_,
                   interface_->energy_,
                   volume_->energy_+interface_->energy_);
            start = chrono::steady_clock::now();
            reconnection_->count_IH_ = 0;
            reconnection_->count_HI_ = 0;
            count_log_++;
        }
        // dump
        if (simulation_time_ - t_start_ + t_roundError > count_dump_ * dump_period_) {
            dumpConfigurationVtk();
            count_dump_++;
        }

        // Euler dynamics
        updateVerticesPosition();

        // reconnect
        if (simulation_time_ - t_start_ + t_roundError > count_reconnect_ * dtr_) {
            reconnection_->start();
            count_reconnect_++;
        }

        simulation_time_ += dt_;
    }

//    for (long int i = 0; i < cells_.size(); i++) {
//        printf("%f\n", cells_[i]->volume_);
//    }

    return 0;
}

int     Run::updateVerticesVelocity() {
    for (long int i = 0; i < vertices_.size(); i++) {
        for (int m = 0; m < 3; m++) {
            vertices_[i]->velocity_[m] = 1.0/eta_ * (vertices_[i]->volumeForce_[m] + vertices_[i]->interfaceForce_[m]);
        }
    }

    return 0;
}

int     Run::updateVerticesPosition() {
    for (long int i = 0; i < vertices_.size(); i++) {
        for (int m = 0; m < 3; m++) {
            vertices_[i]->position_[m] = vertices_[i]->position_[m] + vertices_[i]->velocity_[m] * dt_;
        }
        resetPosition(vertices_[i]->position_);
    }

    return 0;
}

double Run::computeD(double* v1, double* v2) {
    double dx = fabs(v2[0] - v1[0]);
    double dy = fabs(v2[1] - v1[1]);
    while (dx > Lx_/2.0) {
        dx -= Lx_;
    }
    while (dy > Ly_/2.0) {
        dy -= Ly_;
    }

    return sqrt(dx*dx + dy*dy);
}

double Run::computeD(double cx, double cy, double* v) {
    double dx = fabs(v[0] - cx);
    double dy = fabs(v[1] - cy);
    while (dx > Lx_/2.0) {
        dx -= Lx_;
    }
    while (dy > Ly_/2.0) {
        dy -= Ly_;
    }

    return sqrt(dx*dx + dy*dy);
}

int     Run::updatePolygonVertices() {
    // update vertices in polygon
    for (long int i = 0; i < polygons_.size(); i++) {
        polygons_[i]->updateVertices();
    }

    return 0;
}

int     Run::updateVertexEdges() {
    for (long int i = 0; i < vertices_.size(); i++) {
        vertices_[i]->edges_.clear();
    }
    for (long int i = 0; i < edges_.size(); i++) {
        edges_[i]->vertices_[0]->edges_.push_back(edges_[i]);
        edges_[i]->vertices_[1]->edges_.push_back(edges_[i]);
    }

    return 0;
}

int     Run::updateVertexCells() {
    for (long int i = 0; i < vertices_.size(); i++) {
        vertices_[i]->cells_.clear();
    }
    std::vector<Cell *> tpm_cells = cells_;
    tpm_cells.push_back(cellBottom_);
    tpm_cells.push_back(cellTop_);
    for (long int i = 0; i < tpm_cells.size(); i++) {
        Cell * cell = tpm_cells[i];
        for (long int j = 0; j < cell->polygons_.size(); j++) {
            for (long int k = 0; k < cell->polygons_[j]->edges_.size(); k++) {
                for (long int l = 0; l < 2; l++) {
                    Vertex * vertex = cell->polygons_[j]->edges_[k]->vertices_[l];
                    if (std::find(vertex->cells_.begin(), vertex->cells_.end(), cell) == vertex->cells_.end()) {
                        // new cell to be added
                        vertex->cells_.push_back(cell);
                    }
                }
            }
        }
    }

//    for (long int i = 0; i < vertices_.size(); i++) {
//        printf("%d\n", vertices_[i]->cells_.size());
//    }

    return 0;
}

int     Run::updatePolygonCells() {
    for (long int i = 0; i < polygons_.size(); i++) {
        polygons_[i]->cells_.clear();
    }
    for (long int i = 0; i < cells_.size(); i++) {
        for (int j = 0; j < cells_[i]->polygons_.size(); j++) {
            cells_[i]->polygons_[j]->cells_.push_back(cells_[i]);
        }
    }

    return 0;
}

int     Run::updatePolygonType() {
    updatePolygonCells();
    for (long int i = 0; i < polygons_.size(); i++) {
        if (polygons_[i]->cells_.size() > 1) {
            polygons_[i]->cell_cell = true;
        } else {
            polygons_[i]->cell_cell = false;
        }
    }

    return 0;
}

int     Run::updatePolygonDumpType() {
    // 0: red
    // 1: grey
    // 2: blue
    updatePolygonCells();
    for (auto polygon : polygons_) {
        if (polygon->cells_.size() == 1) {
            if (polygon->cells_[0]->growing_) {
                polygon->dumpType = 0;
            } else {
                polygon->dumpType = 1;
            }
        } else if (polygon->cells_.size() == 2) {
            if (polygon->cells_[0]->growing_ != polygon->cells_[1]->growing_) {
                polygon->dumpType = 2;
            } else if (polygon->cells_[0]->growing_) {
                polygon->dumpType = 0;
            } else {
                polygon->dumpType = 1;
            }
        } else {
            printf("polygon %ld has %ld neighboring cells\n", polygon->id_, polygon->cells_.size());
            exit(1);
        }
    }

    return 0;
}

int     Run::updateGeoinfo() {
    // update edge midpoint and length
    for (long int i = 0; i < edges_.size(); i++) {
        edges_[i]->update();
//        printf("%6f\n", run->edges_[i]->length_);
    }
    // update polygon center position
    for (long int i = 0; i < polygons_.size(); i++) {
        polygons_[i]->updateCenter();
    }
    // update polygon type
    updatePolygonType();

    return 0;
}

int     Run::deleteVertex(Vertex * vertex) {
    auto it = find(vertices_.begin(), vertices_.end(), vertex);
    if (it != vertices_.end()) {
//        int index = it - vertices_.begin();
        vertices_.erase(it);
    } else {
        printf("vertex %ld not found in vertices_\n", vertex->id_);
        exit(1);
    }
    delete vertex;

    return 0;
}

int     Run::deleteEdge(Edge * edge) {
    auto it = find(edges_.begin(), edges_.end(), edge);
    if (it != edges_.end()) {
        edges_[it-edges_.begin()]->markToDelete_ = true;
//        edges_.erase(it);
    } else {
        printf("edge %ld not found in edges_\n", edge->id_);
        exit(1);
    }
//    delete edge;

    return 0;
}

int     Run::deletePolygon(Polygon * polygon) {
    auto it = find(polygons_.begin(), polygons_.end(), polygon);
    if (it != polygons_.end()) {
//        int index = it - vertices_.begin();
        polygons_.erase(it);
    } else {
        printf("polygon %ld not found in polygons_\n", polygon->id_);
        exit(1);
    }
    delete polygon;

    return 0;
}

int     Run::resetPosition(double * r) {
    if (fabs(r[0]) > 1e6) {
        printf("%e\n",r[0]);
    }
    if (fabs(r[1]) > 1e6) {
        printf("%e\n",r[1]);
    }
    r[0] = r[0] - Lx_ * floor(r[0] / Lx_);
    r[1] = r[1] - Ly_ * floor(r[1] / Ly_);

    return 0;
}

Edge *  Run::addEdge(Vertex * v0, Vertex * v1) {
    Edge * edge = new Edge(this, count_edges_);
    count_edges_ += 1;
    edges_.push_back(edge);
    if (v0->id_ < v1->id_) {
        edge->vertices_.push_back(v0);
        edge->vertices_.push_back(v1);
    } else {
        edge->vertices_.push_back(v1);
        edge->vertices_.push_back(v0);
    }
    edge->update();
    edge->candidate_ = false;

    return edge;
}

int Run::dumpConfigurationVtk() {
    //////////////////////////////////////////////////////////////////////////////////////
    stringstream filename;
    filename << setw(6) << setfill('0') << (long int)(floor(simulation_time_+0.01*dt_)) << ".sample.vtk";
    ofstream out(filename.str().c_str());
    if (!out.is_open()) {
        cout << "Error opening output file " << filename.str().c_str() << endl;
        exit(1);
    }
    out << "# vtk DataFile Version 2.0" << endl;
    out << "polydata" << endl;
    out << "ASCII" << endl;
    out << "DATASET POLYDATA" << endl;
    out << "POINTS " << vertices_.size() << " double" << endl;
    for (long int i = 0; i < vertices_.size(); i++) {
        // reset vertex id for dumping polygons
        vertices_[i]->id_ = i;
        out << right << setw(12) << scientific << setprecision(5) << vertices_[i]->position_[0];
        out << " " << right << setw(12) << scientific << setprecision(5) << vertices_[i]->position_[1];
        out << " " << right << setw(12) << scientific << setprecision(5) << vertices_[i]->position_[2];
        out << endl;
    }
    out << endl;

//    long int Nedges = 0;
//    for (long int i = 0; i < run->edges_.size(); i++) {
//        if (!run->edges_[i]->crossBoundary()) {
//            Nedges++;
//        }
//    }
//    out << "LINES " << Nedges << " " << 3*Nedges << endl;
//    for (long int i = 0; i < run->edges_.size(); i++) {
//        if (!run->edges_[i]->crossBoundary()) {
//            out << left << setw(6) << 2;
//            for (int j = 0; j < run->edges_[i]->vertices_.size(); j++) {
//                out << " " << left << setw(6) << run->edges_[i]->vertices_[j]->id_;
//            }
//            out << endl;
//        }
//    }
//    out << endl;

    updatePolygonVertices();
    long int Npolygons = 0;
    long int NpolygonVertices = 0;
    for (long int i = 0; i < polygons_.size(); i++) {
        if (!polygons_[i]->crossBoundary()) {
            Npolygons++;
            NpolygonVertices += polygons_[i]->vertices_.size();
        }
    }
    out << "POLYGONS " << Npolygons << " " << Npolygons + NpolygonVertices << endl;
    for (long int i = 0; i < polygons_.size(); i++) {
        if (!polygons_[i]->crossBoundary()) {
            out << left << setw(6) << polygons_[i]->vertices_.size();
            for (int j = 0; j < polygons_[i]->vertices_.size(); j++) {
                out << " " << left << setw(6) << polygons_[i]->vertices_[j]->id_;
            }
            out << endl;
        }
    }
    out << endl;

    updatePolygonDumpType();
    out << "CELL_DATA " << Npolygons << endl;
    out << "SCALARS type int 1" << endl;
    out << "LOOKUP_TABLE default" << endl;
    for (long int i = 0; i < polygons_.size(); i++) {
        if (!polygons_[i]->crossBoundary()) {
            out << left << setw(6) << polygons_[i]->dumpType << endl;
        }
    }
    out << endl;

    out.close();

    return 0;
}