#pragma once


#include "Individual.h"
#include "GADistribution.h"
#include <stdio.h>
#include "BTC.h"
#include "math.h"
#include <iostream>
#include "Matrix.h"
#ifndef mac_version
#include "omp.h"
#endif
#include <vector>

//GUI
class RunTimeWindow;

struct GA_Twiking_parameters
{
    int totnumparams;
	int maxpop=2;
	int nParam;
	int numenhancements, num_enh;
	int nGen;
	int cross_over_type;
	int no_bins;
    bool sens_out = false;
    bool RCGA = false;
    bool readfromgafile;
    double N;
    double pcross=1;
    double pmute=0.02;
    double exponentcoeff=1;
    double shakescale=0.05;
    double shakescalered=0.75;
    char fitnesstype;
};

struct _filenames
{
    string initialpopfilemame;
    string pathname;
    string getfromfilename;
    string outputfilename;
};

using namespace std;

template<class T>
class CGA
{
public:

    double sumfitness;
	double MaxFitness;
    GA_Twiking_parameters GA_params;
    _filenames filenames;
    vector<double> calc_output_percentiles;
    vector<vector<double>> initial_pop;
    vector<double> final_params;
    vector<int> params;
    vector<int> loged;
    vector<int> to_ts;
    vector<double> fixedinputvale;
    vector<double> minval, maxval;
    vector<bool> apply_to_all;
    vector<vector<int>> outcompare;
    vector<CIndividual> Ind;
    vector<CIndividual> Ind_old;
    vector<string> paramname;
    vector<T> Models;
    T Model_out;
    T *Model;

    CGA();
    virtual ~CGA();
    CGA(const CGA &C);
    CGA operator=(CGA &C);
    CGA(string filename, const T&);
    CGA(T*);
    void initialize();
    double getfromoutput(string filename);
    void getinifromoutput(string filename);
    void getinitialpop(string filename);
    int optimize();
    bool SetProperty(const string &varname, const string &value);
    string last_error;
#ifdef Q_version
    void SetRunTimeWindow(RunTimeWindow *_rtw) {rtw=_rtw;}
#endif
private:
    void Setminmax(int a, double minrange, double maxrange, int prec);
    void fitnessdistini();
    void crossover();
    double avgfitness();
    void mutate(double mu);
    void assignfitnesses();
    int maxfitness();
    double variancefitness();
    double stdfitness();
    double avg_actual_fitness();
	void write_to_detailed_GA(string s);
    void setnumpop(int n);
    double avg_inv_actual_fitness();
    int optimize(int nGens, char DefOutPutFileName[]);
    void setnparams(int n);
    void assignfixedvalues();
    void assignrank();
    void assignfitness_rank(double N);
    void shake();
    void crossoverRC();
    void getfromfile(char filename[]);
    void fillfitdist();
    double assignfitnesses(vector<double> inp);
    int getparamno(int i, int ts);
    int get_act_paramno(int i);
    int get_time_series(int i);
    double evaluateforward();
    double evaluateforward_mixed(vector<double> v);
    int current_generation = 0;
    GADistribution fitdist;
    int numberOfThreads;
	// GUI

    #ifdef Q_version
        RunTimeWindow *rtw=nullptr;
    #endif // QT_version


};

#include "GA.hpp"



