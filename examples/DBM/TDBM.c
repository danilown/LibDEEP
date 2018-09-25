#include "deep.h"

int main(int argc, char **argv)
{

    if (argc != 13)
    {
        fprintf(stderr, "\nusage TDBM <training set> <test set> <output results file name> <cross-validation iteration number> \
                <input parameters file> <n_epochs> <batch_size> <number of iterations for Constrastive Divergence> \
                <1 - CD | 2 - PCD | 3 - FPCD> <number of DBM layers> <temperature> <output parameters file name>\n");
        exit(-1);
    }

    int iteration = atoi(argv[4]), i, j, n_epochs = atoi(argv[6]), batch_size = atoi(argv[7]), n_gibbs_sampling = atoi(argv[8]), op = atoi(argv[9]);
    gsl_vector *n_hidden_units = NULL, *eta = NULL, *lambda = NULL, *alpha = NULL, *eta_min = NULL, *eta_max = NULL;
    int n_layers = atoi(argv[10]);
    double temp_eta, temp_lambda, temp_alpha, temp_eta_min, temp_eta_max, t = atof(argv[11]), temp_hidden_units;
    double errorTRAIN, errorTEST;
    char *fileName = argv[5];
    FILE *fp = NULL;
    FILE *fpPar = NULL;
    Dataset *DatasetTrain = NULL, *DatasetTest = NULL;
    DBM *d = NULL;

    Subgraph *Train = NULL, *Test = NULL;
    Train = ReadSubgraph(argv[1]);
    Test = ReadSubgraph(argv[2]);

    DatasetTrain = Subgraph2Dataset(Train);
    DatasetTest = Subgraph2Dataset(Test);

    n_hidden_units = gsl_vector_alloc(n_layers);
    eta = gsl_vector_alloc(n_layers);
    lambda = gsl_vector_alloc(n_layers);
    alpha = gsl_vector_alloc(n_layers);
    eta_min = gsl_vector_alloc(n_layers);
    eta_max = gsl_vector_alloc(n_layers);

    fp = fopen(fileName, "r");
    if (!fp)
    {
        fprintf(stderr, "\nUnable to open file %s.\n", fileName);
        exit(1);
    }

    j = 0;
    for (i = 0; i < n_layers; i++)
    {
        fscanf(fp, "%lf %lf %lf %lf", &temp_hidden_units, &temp_eta, &temp_lambda, &temp_alpha);
        WaiveLibDEEPComment(fp);
        gsl_vector_set(n_hidden_units, i, temp_hidden_units);
        gsl_vector_set(eta, i, temp_eta);
        gsl_vector_set(lambda, i, temp_lambda);
        gsl_vector_set(alpha, i, temp_alpha);
        fscanf(fp, "%lf %lf", &temp_eta_min, &temp_eta_max);
        gsl_vector_set(eta_min, i, temp_eta_min);
        gsl_vector_set(eta_max, i, temp_eta_max);
        WaiveLibDEEPComment(fp);
    }
    fclose(fp);

    fprintf(stderr, "\nCreating and initializing TDBM ... ");
    d = CreateDBM(Train->nfeats, n_hidden_units, Train->nlabels);
    InitializeDBM(d);
    for (i = 0; i < d->n_layers; i++)
    {
        d->m[i]->eta = gsl_vector_get(eta, i);
        d->m[i]->lambda = gsl_vector_get(lambda, i);
        d->m[i]->alpha = gsl_vector_get(alpha, i);
        d->m[i]->eta_min = gsl_vector_get(eta_min, i);
        d->m[i]->eta_max = gsl_vector_get(eta_max, i);
        d->m[i]->t = t;
    }
    fprintf(stderr, "\nOk\n");

    fprintf(stderr, "\nTraining TDBM ...\n");
    errorTRAIN = GreedyPreTrainingDBM(DatasetTrain, d, n_epochs, n_gibbs_sampling, batch_size, op);
    fprintf(stderr, "\nOK\n");

    fprintf(stderr, "\nRunning TDBM for reconstruction ... ");
    errorTEST = BernoulliDBMReconstruction(DatasetTest, d);
    fprintf(stderr, "\nOK\n");

    fprintf(stderr, "\nTraining Error: %lf \nTesting Error: %lf\n\n", errorTRAIN, errorTEST);

    fprintf(stderr, "\nSaving outputs ... ");
    fp = fopen(argv[3], "a");
    fprintf(fp, "\n%d %lf %lf", iteration, errorTRAIN, errorTEST);
    fclose(fp);
    fprintf(stderr, "Ok!\n");

    saveDBMParameters(d, argv[12]);

    DestroyDataset(&DatasetTrain);
    DestroyDataset(&DatasetTest);
    DestroySubgraph(&Train);
    DestroySubgraph(&Test);
    DestroyDBM(&d);
    gsl_vector_free(n_hidden_units);
    gsl_vector_free(lambda);
    gsl_vector_free(eta);
    gsl_vector_free(alpha);
    gsl_vector_free(eta_min);
    gsl_vector_free(eta_max);

    return 0;
}