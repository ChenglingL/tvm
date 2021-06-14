// Copyright June 2021 Tao Zhang @ Shanghai Jiao Tong University.  All Rights Reserved.
// Author: Tao Zhang @ Shanghai Jiao Tong University, zhangtao.scholar@sjtu.edu.cn
// Corresponding author: Jennifer Schwarz @ Syracuse University, jschwarz@physics.syr.edu

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
#include <unordered_map>

#include "Reconnection.h"

using namespace std;

Reconnection::Reconnection(Run * run) {
    run_ = run;
//    Lth_ = 1.0e-3;
    Lth_ = 0.1;
}

int     Reconnection::start() {
    run_->updateGeoinfo();
    // set edge candidate
    for (long int i = 0; i < run_->edges_.size(); i++) {
        if (run_->edges_[i]->length_ < Lth_) {
            run_->edges_[i]->candidate_ = true;
        } else {
            run_->edges_[i]->candidate_ = false;
        }
        run_->edges_[i]->triangle_count_ = 0;
    }
    // set edge triangle_count_
    for (long int i = 0; i < run_->polygons_.size(); i++) {
        if (run_->polygons_[i]->edges_.size() == 3) {
            for (int j = 0; j < 3; j++) {
                run_->polygons_[i]->edges_[j]->triangle_count_ = run_->polygons_[i]->edges_[j]->triangle_count_ + 1;
            }
        }
    }
    // look for H type triangle candidates
    std::vector<Polygon *> triangleCandidates;
    for (long int i = 0; i < run_->polygons_.size(); i++) {
        if (run_->polygons_[i]->checkH()) {
            triangleCandidates.push_back(run_->polygons_[i]);
        }
    }

    // I -> H reconnection
    std::vector<Edge *> tmp_edges = run_->edges_;
    for (long int i = 0; i < tmp_edges.size(); i++) {
        if (tmp_edges[i]->markToDelete_) {
            continue;
        }
        if (!tmp_edges[i]->checkI()) {
            continue;
        }
        I_H(tmp_edges[i], false);
    }

    // H -> I reconnection
    for (long int i = 0; i < triangleCandidates.size(); i++) {
        if (!triangleCandidates[i]->checkH()) {
            continue;
        }
        H_I(triangleCandidates[i]);
    }

    // delete marked edges
//    printf("%ld %ld ", tmp_edges.size(), run_->edges_.size());
    tmp_edges.clear();
    tmp_edges = run_->edges_;
    run_->edges_.clear();
    for (auto edge: tmp_edges) {
        if (edge->markToDelete_) {
            delete edge;
        } else {
            run_->edges_.push_back(edge);
        }
    }
//    printf("%ld\n", run_->edges_.size());

    // update topology and geometry information
    run_->updateVertexCells();
    run_->updateGeoinfo();

    return 0;
}

int Reconnection::I_H(Edge * edge, bool verbose) {
    // locate two vertices: 10 and 11
    Vertex * v10 = edge->vertices_[0];
    Vertex * v11 = edge->vertices_[1];

    // locate five cells: 10-123, 11-456, 1011-1245, 1011-2356, 1011-1346
    // if using finite boundary condition, cellTop_ and cellBottom_ will be the same,
    // and this part should be modified
    if (v10->cells_.size() != 4) {
        printf("Topology Error: vertex %ld has %ld neighboring cells\n", v10->id_, v10->cells_.size());
        exit(1);
    }
    if (v11->cells_.size() != 4) {
        printf("Topology Error: vertex %ld has %ld neighboring cells\n", v11->id_, v11->cells_.size());
        exit(1);
    }
    Cell * c123 = NULL;
    Cell * c456 = NULL;
    Cell * c1245 = NULL;
    Cell * c2356 = NULL;
    Cell * c1346 = NULL;
    std::vector<Cell *> sideCells;
    for (auto cell10 : v10->cells_) {
        if (std::find(v11->cells_.begin(), v11->cells_.end(), cell10) != v11->cells_.end()) {
            sideCells.push_back(cell10);
        } else {
            c123 = cell10;
        }
    }
    if (sideCells.size() != 3) {
        printf("Topology Error: edge %ld has %ld neighboring side cells\n", edge->id_, sideCells.size());
        exit(1);
    }
    c1245 = sideCells[0];
    c2356 = sideCells[1];
    c1346 = sideCells[2];
    for (auto cell11 : v11->cells_) {
        if (std::find(v10->cells_.begin(), v10->cells_.end(), cell11) == v10->cells_.end()) {
            c456 = cell11;
            break;
        }
    }
    if (c456 == NULL) {
        printf("Topology Error: edge %ld has 0 neighboring bottom cells\n", edge->id_);
        exit(1);
    }
    // check if top/bottom pair of cells already have common polygon
    if (commonPolygon(c123, c456) != NULL) {
//        c123->logPolygons("c123");
//        c456->logPolygons("c456");
//        printf("Topology Error: polygon %ld and %ld already have common edge before I->H reconnection\n", c123->id_, c456->id_);
        return 1;
    }
    // locate three side polygons: 1-1011-4, 2-1011-5, 3-1011-6
    Polygon * p14 = commonPolygon(edge, c1245, c1346);
    Polygon * p25 = commonPolygon(edge, c2356, c1245);
    Polygon * p36 = commonPolygon(edge, c1346, c2356);
    // locate three polygons: 10-12, 10-23, 10-13
    Polygon * p12 = commonPolygon(v10, c1245, c123);
    Polygon * p23 = commonPolygon(v10, c2356, c123);
    Polygon * p13 = commonPolygon(v10, c1346, c123);
    // locate three polygons: 11-45, 11-56, 11-46
    Polygon * p45 = commonPolygon(v11, c1245, c456);
    Polygon * p56 = commonPolygon(v11, c2356, c456);
    Polygon * p46 = commonPolygon(v11, c1346, c456);
    if (p14 == NULL || p25 == NULL || p36 == NULL ||
        p12 == NULL || p23 == NULL || p13 == NULL ||
        p45 == NULL || p56 == NULL || p46 == NULL) {
        printf("Topology Error: no common polygon\n");
        exit(1);
    }
    // check if top/bottom pair of polygons already have common edge
    if (commonEdge(p12, p45) != NULL) {
//        p12->logEdges("p12");
//        p45->logEdges("p45");
//        printf("Topology Error: polygon %ld and %ld already have common edge before I->H reconnection\n", p12->id_, p45->id_);
        return 1;
    }
    if (commonEdge(p23, p56) != NULL) {
//        p23->logEdges("p23");
//        p56->logEdges("p56");
//        printf("Topology Error: polygon %ld and %ld already have common edge before I->H reconnection\n", p23->id_, p56->id_);
        return 1;
    }
    if (commonEdge(p13, p46) != NULL) {
//        p13->logEdges("p13");
//        p46->logEdges("p46");
//        printf("Topology Error: polygon %ld and %ld already have common edge before I->H reconnection\n", p13->id_, p46->id_);
        return 1;
    }
    // locate six edges: 10-1, 10-2, 10-3, 11-4, 11-5, 11-6
    Edge * e1 = commonEdge(p12, p13);
    Edge * e2 = commonEdge(p23, p12);
    Edge * e3 = commonEdge(p13, p23);
    Edge * e4 = commonEdge(p45, p46);
    Edge * e5 = commonEdge(p56, p45);
    Edge * e6 = commonEdge(p46, p56);
    if (e1 == NULL || e2 == NULL || e3 == NULL ||
        e4 == NULL || e5 == NULL || e6 == NULL) {
        printf("Topology Error: no common edge\n");
        run_->updateVertexEdges();
        v10->logEdges("v10");
        v11->logEdges("v11");
        v10->logCells("v10");
        v11->logCells("v11");
        c123->logPolygons("c123");
        c456->logPolygons("c456");
        c1245->logPolygons("c1245");
        c1346->logPolygons("c1346");
        c2356->logPolygons("c2356");
        printf("edge %ld\n",edge->id_);
        printf("e1 %ld\n",e1->id_);
        printf("e2 %ld\n",e2->id_);
        printf("e3 %ld\n",e3->id_);
//        printf("e4 %ld\n",e4->id_);
        printf("e5 %ld\n",e5->id_);
        printf("e6 %ld\n",e6->id_);
        p14->logEdges("p14");
        p25->logEdges("p25");
        p36->logEdges("p36");
        p12->logEdges("p12");
        p23->logEdges("p23");
        p13->logEdges("p13");
        p45->logEdges("p45");
        p56->logEdges("p56");
        p46->logEdges("p46");
        run_->updatePolygonVertices();
        std::vector<Polygon *> tmp_polygons = {p14,p25,p36,p12,p23,p13,p45,p46,p56};
        dumpVtk(tmp_polygons, true, true);
        exit(1);
    }
    // locate six vertices: 1, 2, 3, 4, 5, 6
    Vertex * v1 = e1->otherVertex(v10);
    Vertex * v2 = e2->otherVertex(v10);
    Vertex * v3 = e3->otherVertex(v10);
    Vertex * v4 = e4->otherVertex(v11);
    Vertex * v5 = e5->otherVertex(v11);
    Vertex * v6 = e6->otherVertex(v11);
    if (v1 == NULL || v2 == NULL || v3 == NULL ||
        v4 == NULL || v5 == NULL || v6 == NULL) {
        printf("Topology Error: no common vertex\n");
        exit(1);
    }

    if (verbose) {
        run_->updatePolygonVertices();
        std::vector<Polygon *> tmp_polygons = {p14,p25,p36,p12,p23,p13,p45,p46,p56};
        dumpVtk(tmp_polygons, true, true);
    }

    // create vertices 7, 8, 9
    Vertex * v7 = new Vertex(run_, run_->count_vertices_);
    run_->count_vertices_ = run_->count_vertices_ + 1;
    run_->vertices_.push_back(v7);
    Vertex * v8 = new Vertex(run_, run_->count_vertices_);
    run_->count_vertices_ = run_->count_vertices_ + 1;
    run_->vertices_.push_back(v8);
    Vertex * v9 = new Vertex(run_, run_->count_vertices_);
    run_->count_vertices_ = run_->count_vertices_ + 1;
    run_->vertices_.push_back(v9);
    ////////// compute positions of vertices 7, 8, 9 ///////////////
    // r0: midpoint position of edge
    // uT: unit direction vector of edge
    double r0[3];
    double uT[3];
    for (int m = 0; m < 3; m++) {
        r0[m] = edge->center_[m];
        uT[m] = edge->vv_[m];
    }
    double uTL = sqrt(uT[0]*uT[0] + uT[1]*uT[1] + uT[2]*uT[2]);
    for (int m = 0; m < 3; m++) {
        uT[m] = uT[m]/uTL;
    }
    // compute vectors indicating orientation of neighboring polygons
    double w7[3];
    double w8[3];
    double w9[3];
    double r01[3];
    double r02[3];
    double r03[3];
    double r04[3];
    double r05[3];
    double r06[3];
    computeDirection(r0, v1->position_, r01);
    computeDirection(r0, v2->position_, r02);
    computeDirection(r0, v3->position_, r03);
    computeDirection(r0, v4->position_, r04);
    computeDirection(r0, v5->position_, r05);
    computeDirection(r0, v6->position_, r06);
    for (int m = 0; m < 3; m++) {
        w7[m] = 0.5*(r01[m] + r04[m]);
        w8[m] = 0.5*(r02[m] + r05[m]);
        w9[m] = 0.5*(r03[m] + r06[m]);
    }
    // compute vectors indicating orientation of vertices 7, 8, 9
    double wv7[3];
    double wv8[3];
    double wv9[3];
    double dP_w7_uT = 0.;
    double dP_w8_uT = 0.;
    double dP_w9_uT = 0.;
    for (int m = 0; m < 3; m++) {
        dP_w7_uT += w7[m]*uT[m];
        dP_w8_uT += w8[m]*uT[m];
        dP_w9_uT += w9[m]*uT[m];
    }
    for (int m = 0; m < 3; m++) {
        wv7[m] = w7[m] - dP_w7_uT*uT[m];
        wv8[m] = w8[m] - dP_w8_uT*uT[m];
        wv9[m] = w9[m] - dP_w9_uT*uT[m];
    }
    double Lmax = 0.;
    std::vector<double *> tmp_wv = {wv7, wv8, wv9};
    for (auto wv : tmp_wv) {
        double wvL = sqrt(wv[0]*wv[0] + wv[1]*wv[1] + wv[2]*wv[2]);
        if (wvL > Lmax) {
            Lmax = wvL;
        }
    }
    // compute position of vertices 7, 8, 9
    for (int m = 0; m < 3; m++) {
        v7->position_[m] = r0[m] + Lth_/Lmax*wv7[m];
        v8->position_[m] = r0[m] + Lth_/Lmax*wv8[m];
        v9->position_[m] = r0[m] + Lth_/Lmax*wv9[m];
    }
    run_->resetPosition(v7->position_);
    run_->resetPosition(v8->position_);
    run_->resetPosition(v9->position_);
    ////////// compute positions of vertices 7, 8, 9 done  /////////
    // associate cells to vertices 7,8,9
    v7->cells_.push_back(c1245);
    v7->cells_.push_back(c1346);
    v7->cells_.push_back(c123);
    v8->cells_.push_back(c1245);
    v8->cells_.push_back(c2356);
    v8->cells_.push_back(c123);
    v9->cells_.push_back(c2356);
    v9->cells_.push_back(c1346);
    v9->cells_.push_back(c123);
    // create edges 78, 79, 89
    Edge * e78 = run_->addEdge(v7, v8);
    Edge * e79 = run_->addEdge(v7, v9);
    Edge * e89 = run_->addEdge(v8, v9);
    // create polygon 789
    Polygon * p789 = new Polygon(run_, run_->count_polygons_);
    run_->count_polygons_ = run_->count_polygons_ + 1;
    run_->polygons_.push_back(p789);
    p789->edges_.push_back(e78);
    p789->edges_.push_back(e79);
    p789->edges_.push_back(e89);
    // associate p789 to c123 and c456
    c123->polygons_.push_back(p789);
    c456->polygons_.push_back(p789);
    // create edges: 71,82,93,74,85,96
    Edge * e71 = run_->addEdge(v7, v1);
    Edge * e82 = run_->addEdge(v8, v2);
    Edge * e93 = run_->addEdge(v9, v3);
    Edge * e74 = run_->addEdge(v7, v4);
    Edge * e85 = run_->addEdge(v8, v5);
    Edge * e96 = run_->addEdge(v9, v6);
    // update side polygon 1-1011-4
    p14->shrink(edge);
    p14->shrink(e1);
    p14->shrink(e4);
    p14->expand(e71);
    p14->expand(e74);
    // update side polygon 2-1011-5
    p25->shrink(edge);
    p25->shrink(e2);
    p25->shrink(e5);
    p25->expand(e82);
    p25->expand(e85);
    // update side polygon 3-1011-6
    p36->shrink(edge);
    p36->shrink(e3);
    p36->shrink(e6);
    p36->expand(e93);
    p36->expand(e96);
    // update polygon 10-12
    p12->shrink(e1);
    p12->shrink(e2);
    p12->expand(e78);
    p12->expand(e71);
    p12->expand(e82);
    // update polygon 10-23
    p23->shrink(e2);
    p23->shrink(e3);
    p23->expand(e89);
    p23->expand(e82);
    p23->expand(e93);
    // update polygon 10-13
    p13->shrink(e1);
    p13->shrink(e3);
    p13->expand(e79);
    p13->expand(e71);
    p13->expand(e93);
    // update polygon 11-45
    p45->shrink(e4);
    p45->shrink(e5);
    p45->expand(e78);
    p45->expand(e74);
    p45->expand(e85);
    // update polygon 11-56
    p56->shrink(e5);
    p56->shrink(e6);
    p56->expand(e89);
    p56->expand(e85);
    p56->expand(e96);
    // update polygon 11-46
    p46->shrink(e4);
    p46->shrink(e6);
    p46->expand(e79);
    p46->expand(e74);
    p46->expand(e96);

    // delete vertices 10 and 11
    run_->deleteVertex(v10);
    run_->deleteVertex(v11);
    // delete edges
    run_->deleteEdge(edge);
    run_->deleteEdge(e1);
    run_->deleteEdge(e2);
    run_->deleteEdge(e3);
    run_->deleteEdge(e4);
    run_->deleteEdge(e5);
    run_->deleteEdge(e6);

    if (verbose) {
        run_->updatePolygonVertices();
        std::vector<Polygon *> tmp_polygons = {p14,p25,p36,p12,p23,p13,p45,p46,p56};
        dumpVtk(tmp_polygons, true, false);
    }

    return 0;
}

int Reconnection::H_I(Polygon * polygon) {

    return 0;
}

Polygon * Reconnection::commonPolygon(Cell * c1, Cell * c2) {
    std::vector<Polygon *> candidates;
    for (auto polygon : c1->polygons_) {
        if (std::find(c2->polygons_.begin(), c2->polygons_.end(), polygon) != c2->polygons_.end()) {
            candidates.push_back(polygon);
        }
    }
    if (candidates.size() == 1) {
        return candidates[0];
    } else if (candidates.size() == 0) {
        return NULL;
    } else {
        c1->logPolygons("c1");
        c2->logPolygons("c2");
        printf("Topology Error: cell %ld and %ld have more than two common polygons\n", c1->id_, c2->id_);
        exit(1);
    }
}

Polygon * Reconnection::commonPolygon(Edge * edge, Cell * c1, Cell * c2) {
    std::vector<Polygon *> candidates;
    for (auto polygon : c1->polygons_) {
        if (std::find(c2->polygons_.begin(), c2->polygons_.end(), polygon) != c2->polygons_.end()) {
            candidates.push_back(polygon);
        }
    }
    if (candidates.size() == 1) {
        return candidates[0];
    } else if (candidates.size() == 0) {
        return NULL;
    } else {
        // two cells have more than one common polygons
        for (auto polygon : candidates) {
            if (std::find(polygon->edges_.begin(), polygon->edges_.end(), edge) != polygon->edges_.end()) {
                return polygon;
            }
        }
        return NULL;
    }
}

Polygon * Reconnection::commonPolygon(Vertex * vertex, Cell * c1, Cell * c2) {
    std::vector<Polygon *> candidates;
    for (auto polygon : c1->polygons_) {
        if (std::find(c2->polygons_.begin(), c2->polygons_.end(), polygon) != c2->polygons_.end()) {
            candidates.push_back(polygon);
        }
    }
    if (candidates.size() == 1) {
        return candidates[0];
    } else if (candidates.size() == 0) {
        return NULL;
    } else {
        // two cells have more than one common polygons
        for (auto polygon : candidates) {
            polygon->updateVertices();
            if (std::find(polygon->vertices_.begin(), polygon->vertices_.end(), vertex) != polygon->vertices_.end()) {
                return polygon;
            }
        }
        return NULL;
    }
}

Edge * Reconnection::commonEdge(Polygon * p1, Polygon * p2) {
    std::vector<Edge *> candidates;
    for (auto edge : p1->edges_) {
        if (std::find(p2->edges_.begin(), p2->edges_.end(), edge) != p2->edges_.end()) {
            candidates.push_back(edge);
        }
    }
    if (candidates.size() == 1) {
        return candidates[0];
    } else if (candidates.size() == 0) {
        return NULL;
    } else {
        p1->logEdges("e1");
        p2->logEdges("e2");
        printf("Topology Error: polygon %ld and %ld have more than two common edges\n", p1->id_, p2->id_);
        exit(1);
    }
}

int Reconnection::computeDirection(double * r0, double * r1, double * w) {
    for (int m = 0; m < 3; m++) {
        w[m] = r1[m] - r0[m];
    }
    while (w[0] > run_->Lx_/2.0) {
        w[0] = w[0] - run_->Lx_;
    }
    while (w[0] < (-1.0)*run_->Lx_/2.0) {
        w[0] = w[0] + run_->Lx_;
    }
    while (w[1] > run_->Ly_/2.0) {
        w[1] = w[1] - run_->Ly_;
    }
    while (w[1] < (-1.0)*run_->Ly_/2.0) {
        w[1] = w[1] + run_->Ly_;
    }
    double wL = sqrt(w[0]*w[0] + w[1]*w[1] + w[2]*w[2]);
    for (int m = 0; m < 3; m++) {
        w[m] = w[m]/wL;
    }

    return 0;
}

int Reconnection::dumpVtk(std::vector<Polygon *> tmp_polygons, bool IH, bool before) {
    //////////////////////////////////////////////////////////////////////////////////////
    stringstream filename;
    if (IH) {
        if (before) {
            filename << setw(10) << setfill('0') << (long int) (floor(run_->simulation_time_)) << ".IH.before.vtk";
        } else {
            filename << setw(10) << setfill('0') << (long int) (floor(run_->simulation_time_)) << ".IH.after.vtk";
        }
    } else {
        if (before) {
            filename << setw(10) << setfill('0') << (long int) (floor(run_->simulation_time_)) << ".HI.before.vtk";
        } else {
            filename << setw(10) << setfill('0') << (long int) (floor(run_->simulation_time_)) << ".HI.after.vtk";
        }
    }
    ofstream out(filename.str().c_str());
    if (!out.is_open()) {
        cout << "Error opening output file " << filename.str().c_str() << endl;
        exit(1);
    }
    out << "# vtk DataFile Version 2.0" << endl;
    out << "polydata" << endl;
    out << "ASCII" << endl;
    out << "DATASET POLYDATA" << endl;
    out << "POINTS " << run_->vertices_.size() << " double" << endl;
    for (long int i = 0; i < run_->vertices_.size(); i++) {
        // reset vertex id for dumping polygons
        run_->vertices_[i]->id_ = i;
        out << right << setw(12) << scientific << setprecision(5) << run_->vertices_[i]->position_[0];
        out << " " << right << setw(12) << scientific << setprecision(5) << run_->vertices_[i]->position_[1];
        out << " " << right << setw(12) << scientific << setprecision(5) << run_->vertices_[i]->position_[2];
        out << endl;
    }
    out << endl;

    long int Npolygons = 0;
    long int NpolygonVertices = 0;
    for (long int i = 0; i < tmp_polygons.size(); i++) {
        if (!tmp_polygons[i]->crossBoundary()) {
            Npolygons++;
            NpolygonVertices += tmp_polygons[i]->vertices_.size();
        }
    }
    out << "POLYGONS " << Npolygons << " " << Npolygons + NpolygonVertices << endl;
    for (long int i = 0; i < tmp_polygons.size(); i++) {
        if (!tmp_polygons[i]->crossBoundary()) {
            out << left << setw(6) << tmp_polygons[i]->vertices_.size();
            for (int j = 0; j < tmp_polygons[i]->vertices_.size(); j++) {
                out << " " << left << setw(6) << tmp_polygons[i]->vertices_[j]->id_;
            }
            out << endl;
        }
    }
    out << endl;
    out.close();

    return 0;
}
