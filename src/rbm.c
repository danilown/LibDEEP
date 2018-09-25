#include "rbm.h"

/* Allocation and deallocation */

/* It allocates an RBM
Parameters: [n_visible_layer_neurons, n_hidden_layer_neurons, n_labels]
n_visible_layer_neurons: number of visible neurons
n_hidden_layer_neurons: numbder of hidden neurons
n_labels: number of labels */
RBM *CreateRBM(int n_visible_layer_neurons, int n_hidden_layer_neurons, int n_labels)
{
    RBM *m = NULL;

    m = (RBM *)malloc(sizeof(RBM));
    if (!m)
    {
        fprintf(stderr, "\nUnable to alloc memory @CreateRBM.\n");
        exit(-1);
    }

    m->n_visible_layer_neurons = n_visible_layer_neurons;
    m->n_hidden_layer_neurons = n_hidden_layer_neurons;
    m->n_labels = n_labels;
    m->t = 1.0;

    m->v = NULL;
    m->v = gsl_vector_alloc(n_visible_layer_neurons);
    if (!m->v)
    {
        fprintf(stderr, "\nUnable to alloc memory @CreateRBM.\n");
        exit(-1);
    }

    m->a = NULL;
    m->a = gsl_vector_alloc(n_visible_layer_neurons);
    if (!m->a)
    {
        fprintf(stderr, "\nUnable to alloc memory @CreateRBM.\n");
        exit(-1);
    }

    m->h = NULL;
    m->h = gsl_vector_alloc(n_hidden_layer_neurons);
    if (!m->h)
    {
        fprintf(stderr, "\nUnable to alloc memory @CreateRBM.\n");
        exit(-1);
    }

    m->b = NULL;
    m->b = gsl_vector_alloc(n_hidden_layer_neurons);
    if (!m->b)
    {
        fprintf(stderr, "\nUnable to alloc memory @CreateRBM.\n");
        exit(-1);
    }

    m->W = NULL;
    m->W = gsl_matrix_alloc(m->n_visible_layer_neurons, n_hidden_layer_neurons);
    if (!m->W)
    {
        fprintf(stderr, "\nUnable to alloc memory @CreateRBM.\n");
        exit(-1);
    }

    m->c = NULL;
    m->c = gsl_vector_alloc(m->n_labels);
    if (!m->c)
    {
        fprintf(stderr, "\nUnable to alloc memory @CreateRBM.\n");
        exit(-1);
    }

    m->r = NULL;
    m->r = gsl_vector_alloc(n_hidden_layer_neurons);
    gsl_vector_set_all(m->r, 1);
    if (!m->r)
    {
        fprintf(stderr, "\nUnable to alloc memory @CreateRBM.\n");
        exit(-1);
    }

    m->M = NULL;
    m->M = gsl_matrix_alloc(m->n_visible_layer_neurons, n_hidden_layer_neurons);
    gsl_matrix_set_all(m->M, 1);
    if (!m->M)
    {
        fprintf(stderr, "\nUnable to alloc memory @CreateRBM.\n");
        exit(-1);
    }

    m->U = NULL;
    m->U = gsl_matrix_alloc(m->n_labels, n_hidden_layer_neurons);
    if (!m->U)
    {
        fprintf(stderr, "\nUnable to alloc memory @CreateRBM.\n");
        exit(-1);
    }

    return m;
}

/* It allocates a DRBM
Parameters: [n_visible_units, n_hidden_units, n_labels, sigma]
n_visible_units: number of visible units
n_hidden_layen_hidden_unitsrs: number of hidden units
n_labels: number of labels
sigma: array with the variance values associated to each visible unit */
RBM *CreateDRBM(int n_visible_units, int n_hidden_units, int n_labels, gsl_vector *sigma)
{
    RBM *r = NULL;

    r = CreateRBM(n_visible_units, n_hidden_units, n_labels);
    r->sigma = gsl_vector_calloc(sigma->size);
    gsl_vector_memcpy(r->sigma, sigma);

    return r;
}

/* It allocates a new DRBM
Parameters: [n_visible_units, n_hidden_units, n_labels, sigma]
n_visible_units: number of visible units
n_hidden_layen_hidden_unitsrs: number of hidden units
n_labels: number of labels
sigma: array with the variance values associated to each visible unit */
RBM *CreateNewDRBM(int n_visible_units, int n_hidden_units, int n_labels, double *sigma)
{
    int i;
    RBM *r = NULL;

    r = CreateRBM(n_visible_units, n_hidden_units, n_labels);
    r->sigma = gsl_vector_calloc(n_visible_units);
    for (i = 0; i < r->sigma->size; i++)
        gsl_vector_set(r->sigma, i, sigma[i]);

    return r;
}

/* It deallocates an RBM
Parameters: [m]
m: RBM */
void DestroyRBM(RBM **m)
{
    if (*m)
    {
        if ((*m)->v)
            gsl_vector_free((*m)->v);
        if ((*m)->h)
            gsl_vector_free((*m)->h);
        if ((*m)->a)
            gsl_vector_free((*m)->a);
        if ((*m)->b)
            gsl_vector_free((*m)->b);
        if ((*m)->c)
            gsl_vector_free((*m)->c);
        if ((*m)->r)
            gsl_vector_free((*m)->r);
        if ((*m)->W)
            gsl_matrix_free((*m)->W);
        if ((*m)->M)
            gsl_matrix_free((*m)->M);
        if ((*m)->U)
            gsl_matrix_free((*m)->U);
        free(*m);
        *m = NULL;
    }
}

/* It deallocates a DRBM
Parameters: [m]
m: DRBM */
void DestroyDRBM(RBM **m)
{
    if (*m)
    {
        gsl_vector_free((*m)->sigma);
        DestroyRBM(m);
    }
    else
        fprintf(stderr, "\nThere is no DRBM allocated @DestroyDRBM.\n");
}
/**************************/

/* RBM initialization */

/* It initializes the bias of visible units according to Section 8.1
Parameters: [m, D]
m: RBM
D: dataset */
void InitializeBias4VisibleUnits(RBM *m, Dataset *D)
{
    int i, j;
    float *prob = NULL;

    if (D)
    {
        prob = (float *)calloc(D->nfeatures, sizeof(float));
        for (i = 0; i < D->size; i++)
            for (j = 0; j < D->nfeatures; j++)
                prob[j] = prob[j] + (gsl_vector_get(D->sample[i].feature, j) / D->size);
        /* prob[i] means the probability of visible unit i is on */
        for (i = 0; i < m->n_visible_layer_neurons; i++)
        {
            if (prob[i] > 0)
                gsl_vector_set(m->a, i, log(prob[i] / ((1.0 - prob[i]) + 0.0000001)));
            else
                gsl_vector_set(m->a, i, 0.0);
        }
        free(prob);
    }
    else
    {
        fprintf(stderr, "\nDataset not allocated @InitializeBias4VisibleUnits.\n");
        exit(-1);
    }
}

/* It initializes the bias of visible units with small random values [0,1]
Parameters: [m]
m: RBM */
void InitializeBias4VisibleUnitsWithRandomValues(RBM *m)
{
    int i;
    const gsl_rng_type *T;
    gsl_rng *r;

    if (m)
    {
        gsl_rng_env_setup();
        T = gsl_rng_default;
        r = gsl_rng_alloc(T);
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            gsl_vector_set(m->a, i, 0.0);
        gsl_rng_free(r);
    }
    else
    {
        fprintf(stderr, "\nThere is not an RBM allocated @InitializeBias4VisibleUnitsWithRandomValues.\n");
        exit(-1);
    }
}

/* It initializes the bias of hidden units according to Section 8.1
Parameters: [m]
m: RBM */
void InitializeBias4HiddenUnits(RBM *m)
{
    int i;

    if (m)
    {
        for (i = 0; i < m->n_hidden_layer_neurons; i++)
            gsl_vector_set(m->b, i, 0.0);
    }
    else
    {
        fprintf(stderr, "\nThere is not an RBM allocated @InitializeBias4HiddenUnits.\n");
        exit(-1);
    }
}

/* It initializes the bias of hidden dropout units
Parameters: [m, p]
m: RBM
p: hidden neurons dropout rate */
void InitializeBias4DropoutHiddenUnits(RBM *m, double p)
{
    int i;
    const gsl_rng_type *T = NULL;
    gsl_rng *r = NULL;

    if (m)
    {
        srand(time(NULL));
        T = gsl_rng_default;
        r = gsl_rng_alloc(T);
        gsl_rng_set(r, random_seed_deep());
        for (i = 0; i < m->n_hidden_layer_neurons; i++)
            gsl_vector_set(m->r, i, gsl_ran_bernoulli(r, p));
        gsl_rng_free(r);
    }
    else
    {
        fprintf(stderr, "\nThere is not an RBM allocated @InitializeBias4DropoutHiddenUnits.\n");
        exit(-1);
    }
}

/* It initializes the bias of dropconnect
Parameters: [m, p]
m: RBM
p: weight matrix dropconnect rate */
void InitializeBias4DropconnectWeight(RBM *m, double p)
{
    int i, j;
    const gsl_rng_type *T = NULL;
    gsl_rng *r = NULL;

    if (m)
    {
        srand(time(NULL));
        T = gsl_rng_default;
        r = gsl_rng_alloc(T);
        gsl_rng_set(r, random_seed_deep());
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            for (j = 0; j < m->n_hidden_layer_neurons; j++)
                gsl_matrix_set(m->M, i, j, gsl_ran_bernoulli(r, p));
        gsl_rng_free(r);
    }
    else
    {
        fprintf(stderr, "\nThere is not an RBM allocated @InitializeBias4Dropconnect.\n");
        exit(-1);
    }
}

/* It initializes the bias of label units
Parameters: [m]
m: RBM */
void InitializeBias4LabelUnits(RBM *m)
{
    int i;

    if (m)
    {
        for (i = 0; i < m->n_labels; i++)
            gsl_vector_set(m->c, i, 0.0);
    }
    else
    {
        fprintf(stderr, "\nThere is not an RBM allocated @InitializeBias4LabelUnits.\n");
        exit(-1);
    }
}

/* It initializes the weight matrix according to Section 8.1
Parameters: [m]
m: RBM */
void InitializeWeights(RBM *m)
{
    int i, j;
    const gsl_rng_type *T = NULL;
    gsl_rng *r = NULL;

    if (m)
    {
        srand(time(NULL));
        T = gsl_rng_default;
        r = gsl_rng_alloc(T);
        gsl_rng_set(r, random_seed_deep());
        for (i = 0; i < m->n_visible_layer_neurons; i++)
        {
            for (j = 0; j < m->n_hidden_layer_neurons; j++)
            {
                do
                {
                    gsl_matrix_set(m->W, i, j, gsl_ran_gaussian(r, 0.1));
                } while (isnan(gsl_matrix_get(m->W, i, j)));
            }
        }
        gsl_rng_free(r);
    }
    else
    {
        fprintf(stderr, "\nThere is not an RBM allocated @InitializeWeights.\n");
        exit(-1);
    }
}

/* It initializes the label weight matrix
Parameters: [m]
m: RBM */
void InitializeLabelWeights(RBM *m)
{
    int i, j;
    const gsl_rng_type *T = NULL;
    gsl_rng *r = NULL;

    if (m)
    {
        srand(time(NULL));
        T = gsl_rng_default;
        r = gsl_rng_alloc(T);
        gsl_rng_set(r, random_seed_deep());
        for (i = 0; i < m->n_labels; i++)
            for (j = 0; j < m->n_hidden_layer_neurons; j++)
                gsl_matrix_set(m->U, i, j, gsl_ran_gaussian(r, 0.01));
        gsl_rng_free(r);
    }
    else
    {
        fprintf(stderr, "\nThere is not an RBM allocated @InitializeLabelWeights.\n");
        exit(-1);
    }
}

/* It sets the visible layer of a Restricted Boltzmann Machine
Parameters: [m, *visible_layer]
m: RBM
*visible_layer: array of visible layer's ID */
void setVisibleLayer(RBM *m, gsl_vector *visible_layer)
{
    int i;

    if (m)
    {
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            gsl_vector_set(m->v, i, gsl_vector_get(visible_layer, i));
    }
    else
        fprintf(stderr, "\nThe Restricted Boltzmann Machine is not allocated @setVisibleLayer.\n");
}
/**************************/

/* RBM information */

/* It prints the visible units' bias
Parameters: [m]
m: RBM */
void PrintVisibleUnitsBias(RBM *m)
{
    int i;

    if (m)
    {
        fprintf(stderr, "\nShowing visible units' bias ...");
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            fprintf(stderr, "\na[%d]: %f", i, gsl_vector_get(m->a, i));
    }
    else
        fprintf(stderr, "\nRBM not allocated @PrintVisibleUnitsBias.\n");
}

/* It prints the weights
Parameters: [m]
m: RBM */
void PrintWeights(RBM *m)
{
    int i, j;

    if (m)
    {
        fprintf(stderr, "\nShowing weights ...");
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            for (j = 0; j < m->n_hidden_layer_neurons; j++)
                fprintf(stderr, "\nW[%d][%d]: %f", i, j, gsl_matrix_get(m->W, i, j));
    }
    else
        fprintf(stderr, "\nRBM not allocated @PrintWeights.\n");
}

/* It prints the label weights
Parameters: [m]
m: RBM */
void PrintLabelWeights(RBM *m)
{
    int i, j;

    if (m)
    {
        fprintf(stderr, "\nShowing weights ...");
        for (i = 0; i < m->n_labels; i++)
            for (j = 0; j < m->n_hidden_layer_neurons; j++)
                fprintf(stderr, "\nU[%d][%d]: %f", i, j, gsl_matrix_get(m->U, i, j));
    }
    else
        fprintf(stderr, "\nRBM not allocated @PrintLabelWeights.\n");
}

/* It prints the visible units
Parameters: [m]
m: RBM */
void PrintVisibleUnits(RBM *m)
{
    int i, c = 0;

    if (m)
    {
        for (i = 0; i < m->n_visible_layer_neurons; i++)
        {
            fprintf(stderr, "\nv[%d]: %lf", i, gsl_vector_get(m->v, i));
            if (gsl_vector_get(m->v, i) == 1)
                c++;
        }
        fprintf(stderr, "\n\nActive Visible Neurons: %d", c);
    }
    else
        fprintf(stderr, "\nRBM not allocated @PrintVisibleUnits.\n");
}

/* It prints the hidden units
Parameters: [m]
m: RBM */
void PrintHiddenUnits(RBM *m)
{
    int i, c = 0;

    if (m)
    {
        for (i = 0; i < m->n_hidden_layer_neurons; i++)
        {
            fprintf(stderr, "\nh[%d]: %lf", i, gsl_vector_get(m->h, i));
            if (gsl_vector_get(m->h, i) == 1)
                c++;
        }
        fprintf(stderr, "\n\nActive Hidden Neurons: %d", c);
    }
    else
        fprintf(stderr, "\nRBM not allocated @PrintHiddenUnits.\n");
}

/* It prints the hidden dropout units
Parameters: [m]
m: RBM */
void PrintHiddenDropoutUnits(RBM *m)
{
    int i;

    if (m)
    {
        for (i = 0; i < m->n_hidden_layer_neurons; i++)
        {
            fprintf(stderr, "\nr[%d]: %lf", i, gsl_vector_get(m->r, i));
        }
    }
    else
        fprintf(stderr, "\nRBM not allocated @PrintHiddenDropoutUnits.\n");
}

/* It prints the dropconnect matrix mask
Parameters: [m]
m: RBM */
void PrintDropconnectWeight(RBM *m)
{
    int i, j;

    if (m)
    {
        for (i = 0; i < m->n_visible_layer_neurons; i++)
        {
            for (j = 0; j < m->n_hidden_layer_neurons; j++)
            {
                fprintf(stderr, "\ns[%d]: %lf", i, gsl_matrix_get(m->M, i, j));
            }
        }
    }
    else
        fprintf(stderr, "\nRBM not allocated @PrintDropconnectWeight.\n");
}

/* It saves the learned features from the hidden vector neurons
Parameters: [D, m]
D: dataset
m: RBM */
void SaveRBMFeatures(char *s, Dataset *D, RBM *m)
{
    int i, j;
    gsl_vector *h_features = NULL;
    FILE *f = NULL;

    f = fopen(s, "w+");
    fprintf(f, "%d %d %d\n", D->size, D->nlabels, m->n_hidden_layer_neurons);

    for (i = 0; i < D->size; i++)
    {
        h_features = getProbabilityTurningOnHiddenUnit(m, D->sample[i].feature);
        fprintf(f, "%d %d", i, D->sample[i].label);
        for (j = 0; j < m->n_hidden_layer_neurons; j++)
            fprintf(f, " %lf", gsl_vector_get(h_features, j));
        fprintf(f, "\n");
    }

    gsl_vector_free(h_features);
    fclose(f);
}

/* It writes the weight matrix as PGM images without using CV
Parameters: [m, name, indexHiddenUnits, width, height]
m: RBM
name: file name
indexHiddenUnit: ID of hidden unit
width: width value
height: height value */
void SaveWeightsWithoutCV(RBM *m, char *name, int indexHiddenUnit, int width, int height)
{
    int i;
    double min, max, aux;
    FILE *arq;

    arq = fopen(name, "wt");
    fprintf(arq, "P2\n# Comments\n%d %d\n255\n", width, height);

    min = 1000000;
    max = -1000000;

    for (i = 0; i < (width * height); i++)
    {
        if (gsl_matrix_get(m->W, i, indexHiddenUnit) < min)
            min = gsl_matrix_get(m->W, i, indexHiddenUnit);

        if (gsl_matrix_get(m->W, i, indexHiddenUnit) > max)
            max = gsl_matrix_get(m->W, i, indexHiddenUnit);
    }
    for (i = 0; i < (width * height); i++)
    {
        aux = gsl_matrix_get(m->W, i, indexHiddenUnit);
        aux = ((double)aux - (double)min) / ((double)max - (double)min) * 255.0;
        fprintf(arq, "%d ", (int)aux);
    }
    fclose(arq);
}
/**************************/

/* Bernoulli RBM training */

/* It trains a Bernoulli RBM by Constrative Divergence for image reconstruction (binary images)
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data */
double BernoulliRBMTrainingbyContrastiveDivergence(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1 = NULL, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {
                        /* It computes the P(h=1|v1), i.e., it computes h1 */
                        tmp_probh1 = getProbabilityTurningOnHiddenUnit(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        tmp_probvn = getProbabilityTurningOnVisibleUnit(m, m->h);
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);

    return error;
}

/* It trains a Bernoulli RBM by Constrative Divergence with Dropout for image reconstruction (binary images)
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size, p]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data
p: hidden neurons dropout rate */
double BernoulliRBMTrainingbyContrastiveDivergencewithDropout(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size, double p)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1 = NULL, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                /* It computes r for dropping out hidden units */
                InitializeBias4DropoutHiddenUnits(m, p);

                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {
                        /* It computes the P(h=1|v1), i.e., it computes h1 */
                        tmp_probh1 = getProbabilityDroppingVisibleUnitOut4TurningOnHiddenUnit(m, m->r, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        tmp_probvn = getProbabilityDroppingHiddenUnitOut4TurningOnVisibleUnit(m, m->r, m->h);
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityDroppingVisibleUnitOut4TurningOnHiddenUnit(m, m->r, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }

                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);

    return error;
}

/* It trains a Bernoulli RBM with Dropconnect by Constrative Divergence for image reconstruction (binary images)
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size, p]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data
p: dropconnect mask rate */
double BernoulliRBMTrainingbyContrastiveDivergencewithDropconnect(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size, double p)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1 = NULL, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                /* It computes M for dropconnecting weight matrix */
                InitializeBias4DropconnectWeight(m, p);

                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {
                        /* It computes the P(h=1|v1), i.e., it computes h1 */
                        tmp_probh1 = getProbabilityTurningOnHiddenUnit4Dropconnect(m, m->M, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        tmp_probvn = getProbabilityTurningOnVisibleUnit4Dropconnect(m, m->M, m->h);
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit4Dropconnect(m, m->M, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);

    return error;
}

/* It trains a Bernoulli RBM by Persistent Constrative Divergence
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data */
double BernoulliRBMTrainingbyPersistentContrastiveDivergence(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL, *last_probhn = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL, *aux = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    last_probhn = gsl_matrix_calloc(batch_size, m->n_hidden_layer_neurons);
    aux = gsl_vector_calloc(m->n_hidden_layer_neurons);

    error = 0;
    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {
                        /* It computes the P(h=1|v1), i.e., it computes h1 */
                        tmp_probh1 = getProbabilityTurningOnHiddenUnit(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        if ((e == 1) && (n == 1))
                            tmp_probvn = getProbabilityTurningOnVisibleUnit(m, m->h);
                        else
                        {
                            gsl_matrix_get_row(aux, last_probhn, t);
                            tmp_probvn = getProbabilityTurningOnVisibleUnit(m, aux);
                        }
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                            gsl_matrix_set_row(last_probhn, t, probhn); /* It saves the last chain elements of the current training sample */
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);
    gsl_vector_free(aux);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);
    gsl_matrix_free(last_probhn);

    return error;
}

/* It trains a Bernoulli RBM with Dropout by Persistent Constrative Divergence
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size, p]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data
p: hidden units dropout rate */
double BernoulliRBMTrainingbyPersistentContrastiveDivergencewithDropout(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size, double p)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL, *last_probhn = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL, *aux = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    last_probhn = gsl_matrix_calloc(batch_size, m->n_hidden_layer_neurons);
    aux = gsl_vector_calloc(m->n_hidden_layer_neurons);

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                /* It computes r for dropping out hidden units */
                InitializeBias4DropoutHiddenUnits(m, p);

                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {
                        /* It computes the P(h=1|v1), i.e., it computes h1 */
                        tmp_probh1 = getProbabilityDroppingVisibleUnitOut4TurningOnHiddenUnit(m, m->r, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        if ((e == 1) && (n == 1))
                            tmp_probvn = getProbabilityDroppingHiddenUnitOut4TurningOnVisibleUnit(m, m->r, m->h);
                        else
                        {
                            gsl_matrix_get_row(aux, last_probhn, t);
                            tmp_probvn = getProbabilityDroppingHiddenUnitOut4TurningOnVisibleUnit(m, m->r, aux);
                        }
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityDroppingVisibleUnitOut4TurningOnHiddenUnit(m, m->r, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                            gsl_matrix_set_row(last_probhn, t, probhn); /* It saves the last chain elements of the current training sample */
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);
    gsl_vector_free(aux);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);
    gsl_matrix_free(last_probhn);

    return error;
}

/* It trains a Bernoulli RBM with Dropconnect by Persistent Constrative Divergence
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size, p]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data
p: dropconnect rate */
double BernoulliRBMTrainingbyPersistentContrastiveDivergencewithDropconnect(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size, double p)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL, *last_probhn = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL, *aux = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    last_probhn = gsl_matrix_calloc(batch_size, m->n_hidden_layer_neurons);
    aux = gsl_vector_calloc(m->n_hidden_layer_neurons);

    error = 0;
    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                /* It computes M for dropconnecting weight matrix */
                InitializeBias4DropconnectWeight(m, p);
                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {
                        /* It computes the P(h=1|v1), i.e., it computes h1 */
                        tmp_probh1 = getProbabilityTurningOnHiddenUnit4Dropconnect(m, m->M, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        if ((e == 1) && (n == 1))
                            tmp_probvn = getProbabilityTurningOnVisibleUnit4Dropconnect(m, m->M, m->h);
                        else
                        {
                            gsl_matrix_get_row(aux, last_probhn, t);
                            tmp_probvn = getProbabilityTurningOnVisibleUnit4Dropconnect(m, m->M, aux);
                        }
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit4Dropconnect(m, m->M, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                            gsl_matrix_set_row(last_probhn, t, probhn); /* It saves the last chain elements of the current training sample */
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);
    gsl_vector_free(aux);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);
    gsl_matrix_free(last_probhn);

    return error;
}

/* It trains a Bernoulli RBM by Fast Persistent Constrative Divergence
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data */
double BernoulliRBMTrainingbyFastPersistentContrastiveDivergence(Dataset *D, RBM *m, int n_epochs, int n_gibbs_sampling, int batch_size)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum, fast_eta, ratio;
    const gsl_rng_type *T = NULL;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL, *last_probhn = NULL, *fast_W = NULL;
    gsl_matrix *g = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL, *aux = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r = NULL;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    last_probhn = gsl_matrix_calloc(batch_size, m->n_hidden_layer_neurons);
    aux = gsl_vector_calloc(m->n_hidden_layer_neurons);

    /* Fast weights purposes */
    fast_W = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    g = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    fast_eta = m->eta;
    ratio = 19.0 / 20.0;

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_gibbs_sampling; i++)
                    {
                        /* It computes the P(h=1|v1), i.e., it computes h1 */
                        tmp_probh1 = getProbabilityTurningOnHiddenUnit(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        if ((e == 1) && (n == 1))
                            tmp_probvn = getProbabilityTurningOnVisibleUnit4FPCD(m, m->h, fast_W);
                        else
                        {
                            gsl_matrix_get_row(aux, last_probhn, t);
                            tmp_probvn = getProbabilityTurningOnVisibleUnit(m, aux);
                        }
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit4FPCD(m, m->v, fast_W);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_gibbs_sampling)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                            gsl_matrix_set_row(last_probhn, t, probhn); /* It saves the last chain elements of the current training sample */
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates regular weights */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_memcpy(g, CDpos);               /* It copies the gradient to be used in the fast weights computation */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
                                                            /********************************/

            /* It updates fast weights */
            gsl_matrix_scale(g, fast_eta);   /* It computes g*fast_learning_rate */
            gsl_matrix_scale(fast_W, ratio); /* It computes fast_W*19/20 */
            gsl_matrix_add(fast_W, g);       /* It computes fast_W = fast_W*19/20 + gradient*fast_learning_rate */
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);
    gsl_vector_free(aux);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);
    gsl_matrix_free(last_probhn);
    gsl_matrix_free(fast_W);
    gsl_matrix_free(g);

    return error;
}

/* It trains a Bernoulli RBM with Dropout by Fast Persistent Constrative Divergence
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size, p]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data
p: hidden units dropout rate */
double BernoulliRBMTrainingbyFastPersistentContrastiveDivergencewithDropout(Dataset *D, RBM *m, int n_epochs, int n_gibbs_sampling, int batch_size, double p)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum, fast_eta, ratio;
    const gsl_rng_type *T = NULL;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL, *last_probhn = NULL, *fast_W = NULL;
    gsl_matrix *g = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL, *aux = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r = NULL;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    last_probhn = gsl_matrix_calloc(batch_size, m->n_hidden_layer_neurons);
    aux = gsl_vector_calloc(m->n_hidden_layer_neurons);

    /* Fast weights purposes */
    fast_W = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    g = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    fast_eta = m->eta;
    ratio = 19.0 / 20.0;

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                /* It computes r for dropping out hidden units */
                InitializeBias4DropoutHiddenUnits(m, p);

                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_gibbs_sampling; i++)
                    {

                        /* It computes the P(h=1|v1), i.e., it computes h1 */
                        tmp_probh1 = getProbabilityDroppingVisibleUnitOut4TurningOnHiddenUnit(m, m->r, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        if ((e == 1) && (n == 1))
                            tmp_probvn = getProbabilityDroppingHiddenUnitOut4TurningOnVisibleUnit4FPCD(m, m->r, m->h, fast_W);
                        else
                        {
                            gsl_matrix_get_row(aux, last_probhn, t);
                            tmp_probvn = getProbabilityDroppingHiddenUnitOut4TurningOnVisibleUnit(m, m->r, aux);
                        }
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityDroppingVisibleUnitOut4TurningOnHiddenUnit4FPCD(m, m->r, m->v, fast_W);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_gibbs_sampling)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                            gsl_matrix_set_row(last_probhn, t, probhn); /* It saves the last chain elements of the current training sample */
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates regular weights */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_memcpy(g, CDpos);               /* It copies the gradient to be used in the fast weights computation */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
                                                            /********************************/

            /* It updates fast weights */
            gsl_matrix_scale(g, fast_eta);   /* It computes g*fast_learning_rate */
            gsl_matrix_scale(fast_W, ratio); /* It computes fast_W*19/20 */
            gsl_matrix_add(fast_W, g);       /* It computes fast_W = fast_W*19/20 + gradient*fast_learning_rate */
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);
    gsl_vector_free(aux);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);
    gsl_matrix_free(last_probhn);
    gsl_matrix_free(fast_W);
    gsl_matrix_free(g);

    return error;
}

/* It trains a Bernoulli RBM with Dropconnect by Fast Persistent Constrative Divergence
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size, p]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data
p: dropconnect rate */
double BernoulliRBMTrainingbyFastPersistentContrastiveDivergencewithDropconnect(Dataset *D, RBM *m, int n_epochs, int n_gibbs_sampling, int batch_size, double p)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum, fast_eta, ratio;
    const gsl_rng_type *T = NULL;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL, *last_probhn = NULL, *fast_W = NULL;
    gsl_matrix *g = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL, *aux = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r = NULL;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    last_probhn = gsl_matrix_calloc(batch_size, m->n_hidden_layer_neurons);
    aux = gsl_vector_calloc(m->n_hidden_layer_neurons);

    /* Fast weights purposes */
    fast_W = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    g = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    fast_eta = m->eta;
    ratio = 19.0 / 20.0;

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                /* It computes M for dropconnecting weight matrix */
                InitializeBias4DropconnectWeight(m, p);
                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_gibbs_sampling; i++)
                    {
                        /* It computes the P(h=1|v1), i.e., it computes h1 */
                        tmp_probh1 = getProbabilityTurningOnHiddenUnit4Dropconnect(m, m->M, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        if ((e == 1) && (n == 1))
                            tmp_probvn = getProbabilityTurningOnVisibleUnit4FPCD4Dropconnect(m, m->M, m->h, fast_W);
                        else
                        {
                            gsl_matrix_get_row(aux, last_probhn, t);
                            tmp_probvn = getProbabilityTurningOnVisibleUnit4Dropconnect(m, m->M, aux);
                        }
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit4FPCD4Dropconnect(m, m->M, m->v, fast_W);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_gibbs_sampling)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                            gsl_matrix_set_row(last_probhn, t, probhn); /* It saves the last chain elements of the current training sample */
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates regular weights */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_memcpy(g, CDpos);               /* It copies the gradient to be used in the fast weights computation */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
                                                            /********************************/

            /* It updates fast weights */
            gsl_matrix_scale(g, fast_eta);   /* It computes g*fast_learning_rate */
            gsl_matrix_scale(fast_W, ratio); /* It computes fast_W*19/20 */
            gsl_matrix_add(fast_W, g);       /* It computes fast_W = fast_W*19/20 + gradient*fast_learning_rate */
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);
    gsl_vector_free(aux);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);
    gsl_matrix_free(last_probhn);
    gsl_matrix_free(fast_W);
    gsl_matrix_free(g);

    return error;
}

/* It trains a Discriminative Bernoulli RBM by Constrative Divergence for pattern classification
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data */
double DiscriminativeBernoulliRBMTrainingbyContrastiveDivergence(Dataset *D, RBM *m, int n_epochs, int n_gibbs_sampling, int batch_size)
{
    int e, z, j, i, n, n_batches = ceil((float)D->size / batch_size), t, ctr;
    gsl_vector *y0 = NULL, *y1 = NULL, *py1 = NULL, *ph0 = NULL, *ph1 = NULL, *pv1 = NULL, *acc_v0 = NULL, *acc_v1 = NULL;
    gsl_vector *acc_h0 = NULL, *acc_h1 = NULL, *acc_y0 = NULL, *acc_y1 = NULL, *delta_a = NULL, *delta_b = NULL, *delta_c = NULL;
    gsl_matrix *_posW = NULL, *_negW = NULL, *posW = NULL, *negW = NULL, *_posU = NULL, *_negU = NULL, *posU = NULL, *negU = NULL;
    gsl_matrix *tmpW = NULL, *tmpU = NULL, *delta_W = NULL, *delta_U = NULL;
    double sample, error, errorsum, train_error;
    const gsl_rng_type *T = NULL;
    gsl_rng *r = NULL;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    _posW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    posW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    _negW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    negW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    _posU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);
    posU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);
    _negU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);
    negU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);

    delta_W = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    delta_U = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);

    acc_v0 = gsl_vector_calloc(m->n_visible_layer_neurons);
    acc_v1 = gsl_vector_calloc(m->n_visible_layer_neurons);

    acc_h0 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    acc_h1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    acc_y0 = gsl_vector_calloc(m->n_labels);
    acc_y1 = gsl_vector_calloc(m->n_labels);

    delta_a = gsl_vector_calloc(m->n_visible_layer_neurons);
    delta_b = gsl_vector_calloc(m->n_hidden_layer_neurons);
    delta_c = gsl_vector_calloc(m->n_labels);

    train_error = 0;

    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = 0.0;
            gsl_matrix_set_zero(posW);
            gsl_matrix_set_zero(negW);
            gsl_matrix_set_zero(posU);
            gsl_matrix_set_zero(negU);
            gsl_vector_set_zero(acc_v0);
            gsl_vector_set_zero(acc_v1);
            gsl_vector_set_zero(acc_h0);
            gsl_vector_set_zero(acc_h1);
            gsl_vector_set_zero(acc_y0);
            gsl_vector_set_zero(acc_y1);

            for (t = 0; t < batch_size; t++)
            {

                if (z < D->size)
                {
                    ctr++;
                    setVisibleLayer(m, D->sample[z].feature);
                    gsl_vector_add(acc_v0, m->v);
                    y0 = label2binary_gsl_vector(D->sample[z].label, D->nlabels);
                    gsl_vector_add(acc_y0, y0);

                    /* It computes the P(h0|v0,y0) */
                    ph0 = getDiscriminativeProbabilityTurningOnHiddenUnit(m, y0);
                    for (j = 0; j < m->n_hidden_layer_neurons; j++)
                    {
                        sample = gsl_rng_uniform(r);
                        if (gsl_vector_get(ph0, j) > sample)
                            gsl_vector_set(m->h, j, 1.0);
                        else
                            gsl_vector_set(m->h, j, 0.0);
                    }
                    gsl_vector_add(acc_h0, ph0);

                    /* It computes the P(v1|h0) */
                    pv1 = getProbabilityTurningOnVisibleUnit(m, m->h);
                    for (j = 0; j < m->n_visible_layer_neurons; j++)
                    {
                        sample = gsl_rng_uniform(r);
                        if (gsl_vector_get(pv1, j) > sample)
                            gsl_vector_set(m->v, j, 1.0);
                        else
                            gsl_vector_set(m->v, j, 0.0);
                    }
                    gsl_vector_add(acc_v1, m->v);

                    /* It computes the P(y1|h0) */
                    py1 = getDiscriminativeProbabilityLabelUnit(m);
                    y1 = gsl_vector_calloc(py1->size);
                    gsl_vector_set(y1, gsl_vector_max_index(py1), 1.0); /* It samples the class with highest probability */
                    gsl_vector_add(acc_y1, y1);

                    /* It computes the P(h1|y1,v1) */
                    ph1 = getDiscriminativeProbabilityTurningOnHiddenUnit(m, y1);
                    for (j = 0; j < m->n_hidden_layer_neurons; j++)
                    {
                        sample = gsl_rng_uniform(r);
                        if (gsl_vector_get(ph1, j) > sample)
                            gsl_vector_set(m->h, j, 1.0);
                        else
                            gsl_vector_set(m->h, j, 0.0);
                    }
                    gsl_vector_add(acc_h1, ph1);

                    for (i = 0; i < _posW->size1; i++)
                    {
                        for (j = 0; j < _posW->size2; j++)
                        {
                            gsl_matrix_set(_posW, i, j, gsl_vector_get(ph0, j) * gsl_vector_get(D->sample[z].feature, i));
                            gsl_matrix_set(_negW, i, j, gsl_vector_get(ph1, j) * gsl_vector_get(m->v, i));
                        }
                    }

                    for (i = 0; i < _posU->size1; i++)
                    {
                        for (j = 0; j < _posU->size2; j++)
                        {
                            gsl_matrix_set(_posU, i, j, gsl_vector_get(ph0, j) * gsl_vector_get(y0, i));
                            gsl_matrix_set(_negU, i, j, gsl_vector_get(ph1, j) * gsl_vector_get(y1, i));
                        }
                    }

                    gsl_matrix_add(posW, _posW);
                    gsl_matrix_add(negW, _negW);

                    gsl_matrix_add(posU, _posU);
                    gsl_matrix_add(negU, _negU);

                    error += getReconstructionError(y0, py1);

                    gsl_vector_free(y0);
                    gsl_vector_free(y1);
                    gsl_vector_free(py1);
                    gsl_vector_free(ph0);
                    gsl_vector_free(ph1);
                    gsl_vector_free(pv1);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;

            /* Updating W parameter */
            gsl_matrix_sub(posW, negW); /* posW = posW-negW */
            gsl_matrix_scale(posW, 1.0 / ctr);
            gsl_matrix_scale(posW, m->eta); /* posW = eta*posW */
            gsl_matrix_memcpy(tmpW, m->W);
            gsl_matrix_scale(tmpW, -m->lambda);  /* tmp = -lambda*W */
            gsl_matrix_add(posW, tmpW);          /* posW = eta*(posW-negW) - lambda*W */
            gsl_matrix_scale(delta_W, m->alpha); /* delta_W = alpha*delta_W */
            gsl_matrix_add(delta_W, posW);       /* delta_W = eta*(posW-negW) - lambda*W * alpha*delta_W */
            gsl_matrix_add(m->W, delta_W);       /* W = W + delta_W */

            /* Updating U parameter */
            gsl_matrix_sub(posU, negU); /* posU = posU-negU */
            gsl_matrix_scale(posU, 1.0 / ctr);
            gsl_matrix_scale(posU, m->eta); /* posU = eta*posU */
            gsl_matrix_memcpy(tmpU, m->U);
            gsl_matrix_scale(tmpU, -m->lambda);  /* tmp = -lambda*U */
            gsl_matrix_add(posU, tmpU);          /* posU = eta*(posU-negU) -lambda*U */
            gsl_matrix_scale(delta_U, m->alpha); /* delta_U = alpha*delta_U */
            gsl_matrix_add(delta_U, posU);       /* delta_U = eta*(posU-negU) -lambda*U + alpha*delta_U */
            gsl_matrix_add(m->U, delta_U);       /* U = U + delta_U */

            /* Updating a parameter */
            gsl_vector_sub(acc_v0, acc_v1); /* v0 = v0 - v1 */
            gsl_vector_scale(acc_v0, 1.0 / ctr);
            gsl_vector_scale(acc_v0, m->eta);    /* v0 = eta*v0 */
            gsl_vector_scale(delta_a, m->alpha); /* delta_a = alpha*delta_a */
            gsl_vector_add(delta_a, acc_v0);     /* delta_a = eta*(v0-v1) + alpha*delta_a */
            gsl_vector_add(m->a, delta_a);       /* a = a + delta_a */

            /* Updating b parameter */
            gsl_vector_sub(acc_h0, acc_h1); /* h0 = h0 - h1 */
            gsl_vector_scale(acc_h0, 1.0 / ctr);
            gsl_vector_scale(acc_h0, m->eta);    /* h0 = eta*h0 */
            gsl_vector_scale(delta_b, m->alpha); /* delta_b = alpha*delta_b */
            gsl_vector_add(delta_b, acc_h0);     /* delta_b = eta*(h0 - h1) + alpha*delta_b */
            gsl_vector_add(m->b, delta_b);       /* b = b + delta_b */

            /* Updating c parameter */
            gsl_vector_sub(acc_y0, acc_y1); /* y0 = y0 - y1 */
            gsl_vector_scale(acc_y0, 1.0 / ctr);
            gsl_vector_scale(acc_y0, m->eta);    /* y0 = eta*y0 */
            gsl_vector_scale(delta_c, m->alpha); /* delta_c = alpha*delta_c */
            gsl_vector_add(delta_c, acc_y0);     /* delta_c = eta*(y0 - y1) + alpha*delta_c */
            gsl_vector_add(m->c, delta_c);       /* c = c + delta_c */
        }

        fprintf(stderr, "MSE classification error: %lf OK", errorsum / n_batches);
        train_error = errorsum / n_batches;
    }

    gsl_rng_free(r);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(tmpU);
    gsl_matrix_free(_posW);
    gsl_matrix_free(posW);
    gsl_matrix_free(_negW);
    gsl_matrix_free(negW);
    gsl_matrix_free(_posU);
    gsl_matrix_free(_negU);
    gsl_matrix_free(posU);
    gsl_matrix_free(negU);
    gsl_matrix_free(delta_W);
    gsl_matrix_free(delta_U);
    gsl_vector_free(acc_v0);
    gsl_vector_free(acc_v1);
    gsl_vector_free(acc_h0);
    gsl_vector_free(acc_h1);
    gsl_vector_free(acc_y0);
    gsl_vector_free(acc_y1);
    gsl_vector_free(delta_a);
    gsl_vector_free(delta_b);
    gsl_vector_free(delta_c);

    return train_error;
}

/* It trains a Discriminative Bernoulli RBM with Dropout by Constrative Divergence for pattern classification
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size, p]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data
p: hidden neurons dropout rate */
double DiscriminativeBernoulliRBMTrainingbyContrastiveDivergencewithDropout(Dataset *D, RBM *m, int n_epochs, int n_gibbs_sampling, int batch_size, double p)
{
    int e, z, j, i, n, n_batches = ceil((float)D->size / batch_size), t, ctr;
    gsl_vector *y0 = NULL, *y1 = NULL, *py1 = NULL, *ph0 = NULL, *ph1 = NULL, *pv1 = NULL, *acc_v0 = NULL, *acc_v1 = NULL;
    gsl_vector *acc_h0 = NULL, *acc_h1 = NULL, *acc_y0 = NULL, *acc_y1 = NULL, *delta_a = NULL, *delta_b = NULL, *delta_c = NULL;
    gsl_matrix *_posW = NULL, *_negW = NULL, *posW = NULL, *negW = NULL, *_posU = NULL, *_negU = NULL, *posU = NULL, *negU = NULL;
    gsl_matrix *tmpW = NULL, *tmpU = NULL, *delta_W = NULL, *delta_U = NULL;
    double sample, error, errorsum, train_error;
    const gsl_rng_type *T = NULL;
    gsl_rng *r = NULL;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    _posW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    posW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    _negW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    negW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    _posU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);
    posU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);
    _negU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);
    negU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);

    delta_W = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    delta_U = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);

    acc_v0 = gsl_vector_calloc(m->n_visible_layer_neurons);
    acc_v1 = gsl_vector_calloc(m->n_visible_layer_neurons);

    acc_h0 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    acc_h1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    acc_y0 = gsl_vector_calloc(m->n_labels);
    acc_y1 = gsl_vector_calloc(m->n_labels);

    delta_a = gsl_vector_calloc(m->n_visible_layer_neurons);
    delta_b = gsl_vector_calloc(m->n_hidden_layer_neurons);
    delta_c = gsl_vector_calloc(m->n_labels);

    train_error = 0;

    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = 0.0;
            gsl_matrix_set_zero(posW);
            gsl_matrix_set_zero(negW);
            gsl_matrix_set_zero(posU);
            gsl_matrix_set_zero(negU);
            gsl_vector_set_zero(acc_v0);
            gsl_vector_set_zero(acc_v1);
            gsl_vector_set_zero(acc_h0);
            gsl_vector_set_zero(acc_h1);
            gsl_vector_set_zero(acc_y0);
            gsl_vector_set_zero(acc_y1);

            for (t = 0; t < batch_size; t++)
            {
                /* It computes r for dropping out hidden units */
                InitializeBias4DropoutHiddenUnits(m, p);

                if (z < D->size)
                {
                    ctr++;
                    setVisibleLayer(m, D->sample[z].feature);
                    gsl_vector_add(acc_v0, m->v);
                    y0 = label2binary_gsl_vector(D->sample[z].label, D->nlabels);
                    gsl_vector_add(acc_y0, y0);

                    /* It computes the P(h0|v0,y0) */
                    ph0 = getDiscriminativeProbabilityTurningOnHiddenUnit4Dropout(m, m->r, y0);
                    for (j = 0; j < m->n_hidden_layer_neurons; j++)
                    {
                        sample = gsl_rng_uniform(r);
                        if (gsl_vector_get(ph0, j) > sample)
                            gsl_vector_set(m->h, j, 1.0);
                        else
                            gsl_vector_set(m->h, j, 0.0);
                    }
                    gsl_vector_add(acc_h0, ph0);

                    /* It computes the P(v1|h0) */
                    pv1 = getProbabilityDroppingHiddenUnitOut4TurningOnVisibleUnit(m, m->r, m->h);
                    for (j = 0; j < m->n_visible_layer_neurons; j++)
                    {
                        sample = gsl_rng_uniform(r);
                        if (gsl_vector_get(pv1, j) > sample)
                            gsl_vector_set(m->v, j, 1.0);
                        else
                            gsl_vector_set(m->v, j, 0.0);
                    }
                    gsl_vector_add(acc_v1, m->v);

                    /* It computes the P(y1|h0) */
                    py1 = getDiscriminativeProbabilityLabelUnit(m);
                    y1 = gsl_vector_calloc(py1->size);
                    gsl_vector_set(y1, gsl_vector_max_index(py1), 1.0); /* It samples the class with highest probability */
                    gsl_vector_add(acc_y1, y1);

                    /* It computes the P(h1|y1,v1) */
                    ph1 = getDiscriminativeProbabilityTurningOnHiddenUnit4Dropout(m, m->r, y1);
                    for (j = 0; j < m->n_hidden_layer_neurons; j++)
                    {
                        sample = gsl_rng_uniform(r);
                        if (gsl_vector_get(ph1, j) > sample)
                            gsl_vector_set(m->h, j, 1.0);
                        else
                            gsl_vector_set(m->h, j, 0.0);
                    }
                    gsl_vector_add(acc_h1, ph1);

                    for (i = 0; i < _posW->size1; i++)
                    {
                        for (j = 0; j < _posW->size2; j++)
                        {
                            gsl_matrix_set(_posW, i, j, gsl_vector_get(ph0, j) * gsl_vector_get(D->sample[z].feature, i));
                            gsl_matrix_set(_negW, i, j, gsl_vector_get(ph1, j) * gsl_vector_get(m->v, i));
                        }
                    }

                    for (i = 0; i < _posU->size1; i++)
                    {
                        for (j = 0; j < _posU->size2; j++)
                        {
                            gsl_matrix_set(_posU, i, j, gsl_vector_get(ph0, j) * gsl_vector_get(y0, i));
                            gsl_matrix_set(_negU, i, j, gsl_vector_get(ph1, j) * gsl_vector_get(y1, i));
                        }
                    }

                    gsl_matrix_add(posW, _posW);
                    gsl_matrix_add(negW, _negW);

                    gsl_matrix_add(posU, _posU);
                    gsl_matrix_add(negU, _negU);

                    error += getReconstructionError(y0, py1);

                    gsl_vector_free(y0);
                    gsl_vector_free(y1);
                    gsl_vector_free(py1);
                    gsl_vector_free(ph0);
                    gsl_vector_free(ph1);
                    gsl_vector_free(pv1);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;

            /* Updating W parameter */
            gsl_matrix_sub(posW, negW); /* posW = posW-negW */
            gsl_matrix_scale(posW, 1.0 / ctr);
            gsl_matrix_scale(posW, m->eta); /* posW = eta*posW */
            gsl_matrix_memcpy(tmpW, m->W);
            gsl_matrix_scale(tmpW, -m->lambda);  /* tmp = -lambda*W */
            gsl_matrix_add(posW, tmpW);          /* posW = eta*(posW-negW) - lambda*W */
            gsl_matrix_scale(delta_W, m->alpha); /* delta_W = alpha*delta_W */
            gsl_matrix_add(delta_W, posW);       /* delta_W = eta*(posW-negW) - lambda*W * alpha*delta_W */
            gsl_matrix_add(m->W, delta_W);       /* W = W + delta_W */

            /* Updating U parameter */
            gsl_matrix_sub(posU, negU); /* posU = posU-negU */
            gsl_matrix_scale(posU, 1.0 / ctr);
            gsl_matrix_scale(posU, m->eta); /* posU = eta*posU */
            gsl_matrix_memcpy(tmpU, m->U);
            gsl_matrix_scale(tmpU, -m->lambda);  /* tmp = -lambda*U */
            gsl_matrix_add(posU, tmpU);          /* posU = eta*(posU-negU) -lambda*U */
            gsl_matrix_scale(delta_U, m->alpha); /* delta_U = alpha*delta_U */
            gsl_matrix_add(delta_U, posU);       /* delta_U = eta*(posU-negU) -lambda*U + alpha*delta_U */
            gsl_matrix_add(m->U, delta_U);       /* U = U + delta_U */

            /* Updating a parameter */
            gsl_vector_sub(acc_v0, acc_v1); /* v0 = v0 - v1 */
            gsl_vector_scale(acc_v0, 1.0 / ctr);
            gsl_vector_scale(acc_v0, m->eta);    /* v0 = eta*v0 */
            gsl_vector_scale(delta_a, m->alpha); /* delta_a = alpha*delta_a */
            gsl_vector_add(delta_a, acc_v0);     /* delta_a = eta*(v0-v1) + alpha*delta_a */
            gsl_vector_add(m->a, delta_a);       /* a = a + delta_a */

            /* Updating b parameter */
            gsl_vector_sub(acc_h0, acc_h1); /* h0 = h0 - h1 */
            gsl_vector_scale(acc_h0, 1.0 / ctr);
            gsl_vector_scale(acc_h0, m->eta);    /* h0 = eta*h0 */
            gsl_vector_scale(delta_b, m->alpha); /* delta_b = alpha*delta_b */
            gsl_vector_add(delta_b, acc_h0);     /* delta_b = eta*(h0 - h1) + alpha*delta_b */
            gsl_vector_add(m->b, delta_b);       /* b = b + delta_b */

            /* Updating c parameter */
            gsl_vector_sub(acc_y0, acc_y1); /* y0 = y0 - y1 */
            gsl_vector_scale(acc_y0, 1.0 / ctr);
            gsl_vector_scale(acc_y0, m->eta);    /* y0 = eta*y0 */
            gsl_vector_scale(delta_c, m->alpha); /* delta_c = alpha*delta_c */
            gsl_vector_add(delta_c, acc_y0);     /* delta_c = eta*(y0 - y1) + alpha*delta_c */
            gsl_vector_add(m->c, delta_c);       /* c = c + delta_c */
        }

        fprintf(stderr, "MSE classification error: %lf OK", errorsum / n_batches);
        train_error = errorsum / n_batches;
    }

    gsl_rng_free(r);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(tmpU);
    gsl_matrix_free(_posW);
    gsl_matrix_free(posW);
    gsl_matrix_free(_negW);
    gsl_matrix_free(negW);
    gsl_matrix_free(_posU);
    gsl_matrix_free(_negU);
    gsl_matrix_free(posU);
    gsl_matrix_free(negU);
    gsl_matrix_free(delta_W);
    gsl_matrix_free(delta_U);
    gsl_vector_free(acc_v0);
    gsl_vector_free(acc_v1);
    gsl_vector_free(acc_h0);
    gsl_vector_free(acc_h1);
    gsl_vector_free(acc_y0);
    gsl_vector_free(acc_y1);
    gsl_vector_free(delta_a);
    gsl_vector_free(delta_b);
    gsl_vector_free(delta_c);

    return train_error;
}

/* It trains a Bernoulli RBM by Constrative Divergence for image reconstruction regarding DBMs at the bottom layer
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data */
double Bernoulli_TrainingRBMbyCD4DBM_BottomLayer(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {
                        /* It computes the P(h=1|v1), i.e., it computes h1 */
                        tmp_probh1 = getProbabilityTurningOnHiddenUnit4DBM(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        tmp_probvn = getProbabilityTurningOnVisibleUnit(m, m->h);
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit4DBM(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); //it averages CDpos
            gsl_matrix_scale(CDneg, 1.0 / batch_size); //it averages CDneg
            gsl_matrix_sub(CDpos, CDneg);              //it performs CDpos-CDneg
            gsl_matrix_scale(CDpos, m->eta);           // it performs eta*(CDpos-CDneg)
            gsl_matrix_scale(tmpW, m->alpha);          // it performs W' = alpha*W' (momentum)
            gsl_matrix_memcpy(auxW, m->W);             // it performs auxW = W
            gsl_matrix_scale(auxW, -m->lambda);        // it performs auxW = -lambda*W (weight decay)
            gsl_matrix_add(tmpW, auxW);                // it performs W' = W-lambda*W' (weight decay)
            gsl_matrix_add(tmpW, CDpos);               // it performs W' = W'+eta*(CDpos-CDneg)
            gsl_matrix_add(m->W, tmpW);                // it performs W = W+W'

            gsl_vector_scale(v1, 1.0 / batch_size); //it averages v1
            gsl_vector_scale(vn, 1.0 / batch_size); //it averages vn
            gsl_vector_sub(v1, vn);                 // it performs v1-vn
            gsl_vector_scale(v1, m->eta);           //it performs eta*(v1-vn)
            gsl_vector_scale(tmpa, m->alpha);       // it performs a'= alpha*a'
            gsl_vector_add(tmpa, v1);               //it performs a' = alpha*a' + eta(v1-vn)
            gsl_vector_add(m->a, tmpa);             //it performs a = a + a'*/

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); //it averages P(h1 = 1|v1)
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); //it averages P(h2 = 1|v2)
            gsl_vector_scale(tmpb, m->alpha);               //it performs b'= alpha*b'
            gsl_vector_sub(ctr_probh1, ctr_probhn);         //it performs P(h1 = 1|v1) - P(h2 = 1|v2)
            gsl_vector_scale(ctr_probh1, m->eta);           //it performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2))
            gsl_vector_add(tmpb, ctr_probh1);               //it performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2))
            gsl_vector_add(m->b, tmpb);                     // it performs b = b + b'
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);

    return error;
}

/* It trains a Bernoulli RBM with Dropout by Constrative Divergence for image reconstruction regarding DBMs at the bottom layer
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size, p]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data
p: hidden units dropout rate */
double Bernoulli_TrainingRBMbyCD4DBM_BottomLayerwithDropout(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size, double p)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                /* It computes r for dropping out hidden units */
                InitializeBias4DropoutHiddenUnits(m, p);

                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {
                        /* It computes the P(h=1|v1), i.e., it computes h1 */
                        tmp_probh1 = getProbabilityDroppingVisibleUnitOut4TurningOnHiddenUnit4DBM(m, m->r, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        tmp_probvn = getProbabilityDroppingHiddenUnitOut4TurningOnVisibleUnit(m, m->r, m->h);
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityDroppingVisibleUnitOut4TurningOnHiddenUnit4DBM(m, m->r, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); //it averages CDpos
            gsl_matrix_scale(CDneg, 1.0 / batch_size); //it averages CDneg
            gsl_matrix_sub(CDpos, CDneg);              //it performs CDpos-CDneg
            gsl_matrix_scale(CDpos, m->eta);           // it performs eta*(CDpos-CDneg)
            gsl_matrix_scale(tmpW, m->alpha);          // it performs W' = alpha*W' (momentum)
            gsl_matrix_memcpy(auxW, m->W);             // it performs auxW = W
            gsl_matrix_scale(auxW, -m->lambda);        // it performs auxW = -lambda*W (weight decay)
            gsl_matrix_add(tmpW, auxW);                // it performs W' = W-lambda*W' (weight decay)
            gsl_matrix_add(tmpW, CDpos);               // it performs W' = W'+eta*(CDpos-CDneg)
            gsl_matrix_add(m->W, tmpW);                // it performs W = W+W'

            gsl_vector_scale(v1, 1.0 / batch_size); //it averages v1
            gsl_vector_scale(vn, 1.0 / batch_size); //it averages vn
            gsl_vector_sub(v1, vn);                 // it performs v1-vn
            gsl_vector_scale(v1, m->eta);           //it performs eta*(v1-vn)
            gsl_vector_scale(tmpa, m->alpha);       // it performs a'= alpha*a'
            gsl_vector_add(tmpa, v1);               //it performs a' = alpha*a' + eta(v1-vn)
            gsl_vector_add(m->a, tmpa);             //it performs a = a + a'*/

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); //it averages P(h1 = 1|v1)
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); //it averages P(h2 = 1|v2)
            gsl_vector_scale(tmpb, m->alpha);               //it performs b'= alpha*b'
            gsl_vector_sub(ctr_probh1, ctr_probhn);         //it performs P(h1 = 1|v1) - P(h2 = 1|v2)
            gsl_vector_scale(ctr_probh1, m->eta);           //it performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2))
            gsl_vector_add(tmpb, ctr_probh1);               //it performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2))
            gsl_vector_add(m->b, tmpb);                     // it performs b = b + b'
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);

    return error;
}

/* It trains a Bernoulli RBM with Dropconnect by Constrative Divergence for image reconstruction regarding DBMs at the bottom layer
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size, p]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data
p: dropconnect mask rate */
double Bernoulli_TrainingRBMbyCD4DBM_BottomLayerwithDropconnect(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size, double p)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                /* It computes r for dropping connect weight matrix */
                InitializeBias4DropconnectWeight(m, p);

                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {
                        /* It computes the P(h=1|v1), i.e., it computes h1 */
                        tmp_probh1 = getProbabilityTurningOnHiddenUnit4DBM4Dropconnect(m, m->M, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        tmp_probvn = getProbabilityTurningOnVisibleUnit4Dropconnect(m, m->M, m->h);
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit4DBM4Dropconnect(m, m->M, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); //it averages CDpos
            gsl_matrix_scale(CDneg, 1.0 / batch_size); //it averages CDneg
            gsl_matrix_sub(CDpos, CDneg);              //it performs CDpos-CDneg
            gsl_matrix_scale(CDpos, m->eta);           // it performs eta*(CDpos-CDneg)
            gsl_matrix_scale(tmpW, m->alpha);          // it performs W' = alpha*W' (momentum)
            gsl_matrix_memcpy(auxW, m->W);             // it performs auxW = W
            gsl_matrix_scale(auxW, -m->lambda);        // it performs auxW = -lambda*W (weight decay)
            gsl_matrix_add(tmpW, auxW);                // it performs W' = W-lambda*W' (weight decay)
            gsl_matrix_add(tmpW, CDpos);               // it performs W' = W'+eta*(CDpos-CDneg)
            gsl_matrix_add(m->W, tmpW);                // it performs W = W+W'

            gsl_vector_scale(v1, 1.0 / batch_size); //it averages v1
            gsl_vector_scale(vn, 1.0 / batch_size); //it averages vn
            gsl_vector_sub(v1, vn);                 // it performs v1-vn
            gsl_vector_scale(v1, m->eta);           //it performs eta*(v1-vn)
            gsl_vector_scale(tmpa, m->alpha);       // it performs a'= alpha*a'
            gsl_vector_add(tmpa, v1);               //it performs a' = alpha*a' + eta(v1-vn)
            gsl_vector_add(m->a, tmpa);             //it performs a = a + a'*/

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); //it averages P(h1 = 1|v1)
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); //it averages P(h2 = 1|v2)
            gsl_vector_scale(tmpb, m->alpha);               //it performs b'= alpha*b'
            gsl_vector_sub(ctr_probh1, ctr_probhn);         //it performs P(h1 = 1|v1) - P(h2 = 1|v2)
            gsl_vector_scale(ctr_probh1, m->eta);           //it performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2))
            gsl_vector_add(tmpb, ctr_probh1);               //it performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2))
            gsl_vector_add(m->b, tmpb);                     // it performs b = b + b'
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);

    return error;
}

/* It trains a Bernoulli RBM by Constrative Divergence for image reconstruction regarding DBMs at the top layer
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data */
double Bernoulli_TrainingRBMbyCD4DBM_TopLayer(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {

                        /* It computes the P(h_n=1|h_(n-1)) -> Equation 25 */
                        tmp_probh1 = getProbabilityTurningOnHiddenUnit(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(h_(n-1)=1|h_n)) -> Equation 24 */
                        tmp_probvn = getProbabilityTurningOnVisibleUnit4DBM(m, m->h);
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h_n=1|h_(n-1)) -> Equation 25 */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);

    return error;
}

/* It trains a Bernoulli RBM with Dropout by Constrative Divergence for image reconstruction regarding DBMs at the top layer
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size, p]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data
p: hidden neurons dropout rate */
double Bernoulli_TrainingRBMbyCD4DBM_TopLayerwithDropout(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size, double p)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                /* It computes r for dropping out hidden units */
                InitializeBias4DropoutHiddenUnits(m, p);

                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {

                        /* It computes the P(h_n=1|h_(n-1)) -> Equation 25 */
                        tmp_probh1 = getProbabilityDroppingVisibleUnitOut4TurningOnHiddenUnit(m, m->r, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(h_(n-1)=1|h_n)) -> Equation 24 */
                        tmp_probvn = getProbabilityDroppingHiddenUnitOut4TurningOnVisibleUnit4DBM(m, m->r, m->h);
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h_n=1|h_(n-1)) -> Equation 25 */
                        tmp_probhn = getProbabilityDroppingVisibleUnitOut4TurningOnHiddenUnit(m, m->r, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);

    return error;
}

/* It trains a Bernoulli RBM with Dropconnect by Constrative Divergence for image reconstruction regarding DBMs at the top layer
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size, p]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data
p: dropconnect mask rate */
double Bernoulli_TrainingRBMbyCD4DBM_TopLayerwithDropconnect(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size, double p)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                /* It computes r for dropping connect mask weight */
                InitializeBias4DropconnectWeight(m, p);

                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {

                        /* It computes the P(h_n=1|h_(n-1)) -> Equation 25 */
                        tmp_probh1 = getProbabilityTurningOnHiddenUnit4Dropconnect(m, m->M, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(h_(n-1)=1|h_n)) -> Equation 24 */
                        tmp_probvn = getProbabilityTurningOnVisibleUnit4DBM4Dropconnect(m, m->M, m->h);
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h_n=1|h_(n-1)) -> Equation 25 */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit4Dropconnect(m, m->M, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);

    return error;
}

/* It trains a Bernoulli RBM by Constrative Divergence for image reconstruction regarding DBMs at the intermediate layers
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data */
double Bernoulli_TrainingRBMbyCD4DBM_IntermediateLayers(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {

                        /* It computes the P(h_k=1|h_(k-1)) -> Equation 27 */
                        tmp_probh1 = getProbabilityTurningOnHiddenUnit4DBM(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(h_(k-1)=1|h_k) -> Equation 26 */
                        tmp_probvn = getProbabilityTurningOnVisibleUnit4DBM(m, m->h);
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h_k=1|h_(k-1)) -> Equation 27 */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit4DBM(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);

    return error;
}

/* It trains a Bernoulli RBM with Dropout by Constrative Divergence for image reconstruction regarding DBMs at the intermediate layers
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size, p]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data
p: hidden neurons dropout rate */
double Bernoulli_TrainingRBMbyCD4DBM_IntermediateLayerswithDropout(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size, double p)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                /* It computes r for dropping out hidden units */
                InitializeBias4DropoutHiddenUnits(m, p);

                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {

                        /* It computes the P(h_k=1|h_(k-1)) -> Equation 27 */
                        tmp_probh1 = getProbabilityDroppingVisibleUnitOut4TurningOnHiddenUnit4DBM(m, m->r, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(h_(k-1)=1|h_k) -> Equation 26 */
                        tmp_probvn = getProbabilityDroppingHiddenUnitOut4TurningOnVisibleUnit4DBM(m, m->r, m->h);
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h_k=1|h_(k-1)) -> Equation 27 */
                        tmp_probhn = getProbabilityDroppingVisibleUnitOut4TurningOnHiddenUnit4DBM(m, m->r, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);

    return error;
}

/* It trains a Bernoulli RBM with Dropconnect by Constrative Divergence for image reconstruction regarding DBMs at the intermediate layers
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size, p]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data
p: hidden neurons dropout rate */
double Bernoulli_TrainingRBMbyCD4DBM_IntermediateLayerswithDropconnect(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size, double p)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                /* It computes r for dropping connect mask weight */
                InitializeBias4DropconnectWeight(m, p);

                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {

                        /* It computes the P(h_k=1|h_(k-1)) -> Equation 27 */
                        tmp_probh1 = getProbabilityTurningOnHiddenUnit4DBM4Dropconnect(m, m->M, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(h_(k-1)=1|h_k) -> Equation 26 */
                        tmp_probvn = getProbabilityTurningOnVisibleUnit4DBM4Dropconnect(m, m->M, m->h);
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h_k=1|h_(k-1)) -> Equation 27 */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit4DBM4Dropconnect(m, m->M, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);

    return error;
}

/* It trains a Bernoulli RBM by Persistent Constrative Divergence for image reconstruction regarding DBMs at the bottom layer
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data */
double Bernoulli_TrainingRBMbyPCD4DBM_BottomLayer(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL, *last_probhn = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL, *aux = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    last_probhn = gsl_matrix_calloc(batch_size, m->n_hidden_layer_neurons);
    aux = gsl_vector_calloc(m->n_hidden_layer_neurons);

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {

                        /* It computes the P(h=1|v1), i.e., it computes h1 */
                        tmp_probh1 = getProbabilityTurningOnHiddenUnit4DBM(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        if ((e == 1) && (n == 1))
                            tmp_probvn = getProbabilityTurningOnVisibleUnit(m, m->h);
                        else
                        {
                            gsl_matrix_get_row(aux, last_probhn, t);
                            tmp_probvn = getProbabilityTurningOnVisibleUnit(m, m->h);
                        }
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit4DBM(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                            gsl_matrix_set_row(last_probhn, t, probhn); /* It saves the last chain elements of the current training sample */
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);
    gsl_vector_free(aux);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);
    gsl_matrix_free(last_probhn);

    return error;
}

/* It trains a Bernoulli RBM by Persistent Constrative Divergence for image reconstruction regarding DBMs at the top layer
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data */
double Bernoulli_TrainingRBMbyPCD4DBM_TopLayer(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL, *last_probhn = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL, *aux = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    last_probhn = gsl_matrix_calloc(batch_size, m->n_hidden_layer_neurons);
    aux = gsl_vector_calloc(m->n_hidden_layer_neurons);

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {

                        /* It computes the P(h=1|v1), i.e., it computes h1 */
                        tmp_probh1 = getProbabilityTurningOnHiddenUnit(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        if ((e == 1) && (n == 1))
                            tmp_probvn = getProbabilityTurningOnVisibleUnit4DBM(m, m->h);
                        else
                        {
                            gsl_matrix_get_row(aux, last_probhn, t);
                            tmp_probvn = getProbabilityTurningOnVisibleUnit4DBM(m, m->h);
                        }
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                            gsl_matrix_set_row(last_probhn, t, probhn); /* It saves the last chain elements of the current training sample */
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);
    gsl_vector_free(aux);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);
    gsl_matrix_free(last_probhn);

    return error;
}

/* It trains a Bernoulli RBM by Constrative Divergence for image reconstruction regarding DBMs at the intermediate layers
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data */
double Bernoulli_TrainingRBMbyPCD4DBM_IntermediateLayers(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr;
    double error, sample, errorsum, pl, plsum;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL, *last_probhn = NULL;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL, *aux = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL;
    gsl_rng *r;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    last_probhn = gsl_matrix_calloc(batch_size, m->n_hidden_layer_neurons);
    aux = gsl_vector_calloc(m->n_hidden_layer_neurons);

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = plsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {

            ctr = 0;
            error = pl = 0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);

            for (t = 0; t < batch_size; t++)
            {
                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);

                    /* It accumulates v1 */
                    gsl_vector_add(v1, m->v);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {
                        /* It computes the P(h=1|v1), i.e., it computes h1 */
                        tmp_probh1 = getProbabilityTurningOnHiddenUnit4DBM(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probh1, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == 1)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probh1, tmp_probh1);
                            gsl_vector_add(ctr_probh1, probh1);
                        }
                        gsl_vector_free(tmp_probh1);

                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        if ((e == 1) && (n == 1))
                            tmp_probvn = getProbabilityTurningOnVisibleUnit4DBM(m, m->h);
                        else
                        {
                            gsl_matrix_get_row(aux, last_probhn, t);
                            tmp_probvn = getProbabilityTurningOnVisibleUnit4DBM(m, m->h);
                        }
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probvn, j) >= sample)
                                gsl_vector_set(m->v, j, 1.0);
                            else
                                gsl_vector_set(m->v, j, 0.0);
                        }

                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit4DBM(m, m->v);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) >= sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }
                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, tmp_probvn);
                            gsl_matrix_set_row(last_probhn, t, probhn); /* It saves the last chain elements of the current training sample */
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    gsl_vector_add(vn, m->v);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, gsl_vector_get(D->sample[z].feature, i) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, gsl_vector_get(m->v, i) * gsl_vector_get(probhn, j));
                        }
                    }

                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            /* It updates RBM parameters */
            gsl_matrix_scale(CDpos, 1.0 / batch_size); /* It averages CDpos */
            gsl_matrix_scale(CDneg, 1.0 / batch_size); /* It averages CDneg */
            gsl_matrix_sub(CDpos, CDneg);              /* It performs CDpos-CDneg */
            gsl_matrix_scale(CDpos, m->eta);           /* It performs eta*(CDpos-CDneg) */
            gsl_matrix_scale(tmpW, m->alpha);          /* It performs W' = alpha*W' (momentum) */
            gsl_matrix_memcpy(auxW, m->W);             /* It performs auxW = W */
            gsl_matrix_scale(auxW, -m->lambda);        /* It performs auxW = -lambda*W (weight decay) */
            gsl_matrix_add(tmpW, auxW);                /* It performs W' = W-lambda*W' (weight decay) */
            gsl_matrix_add(tmpW, CDpos);               /* It performs W' = W'+eta*(CDpos-CDneg) */
            gsl_matrix_add(m->W, tmpW);                /* It performs W = W+W' */

            gsl_vector_scale(v1, 1.0 / batch_size); /* It averages v1 */
            gsl_vector_scale(vn, 1.0 / batch_size); /* It averages vn */
            gsl_vector_sub(v1, vn);                 /* It performs v1-vn */
            gsl_vector_scale(v1, m->eta);           /* It performs eta*(v1-vn) */
            gsl_vector_scale(tmpa, m->alpha);       /* It performs a'= alpha*a' */
            gsl_vector_add(tmpa, v1);               /* It performs a' = alpha*a' + eta(v1-vn) */
            gsl_vector_add(m->a, tmpa);             /* It performs a = a + a' */

            gsl_vector_scale(ctr_probh1, 1.0 / batch_size); /* It averages P(h1 = 1|v1) */
            gsl_vector_scale(ctr_probhn, 1.0 / batch_size); /* It averages P(h2 = 1|v2) */
            gsl_vector_scale(tmpb, m->alpha);               /* It performs b'= alpha*b' */
            gsl_vector_sub(ctr_probh1, ctr_probhn);         /* It performs P(h1 = 1|v1) - P(h2 = 1|v2) */
            gsl_vector_scale(ctr_probh1, m->eta);           /* It performs eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(tmpb, ctr_probh1);               /* It performs b' = alpha*b' + eta*(P(h1 = 1|v1) - P(h2 = 1|v2)) */
            gsl_vector_add(m->b, tmpb);                     /* It performs b = b + b' */
            /********************************/
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "    -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        m->eta = m->eta_max - ((m->eta_max - m->eta_min) / n_epochs) * e;

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);
    gsl_vector_free(aux);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);
    gsl_matrix_free(last_probhn);

    return error;
}
/**************************/

/* Gaussian-Bernoulli RBM Training */

/* It trains a Gaussian-Bernoulli RBM by Constrative Divergence for image reconstruction (binary images)
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data */
double GaussianBernoulliRBMTrainingbyContrastiveDivergence(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr, w, line;
    double error, sample, errorsum, pl, plsum, v_std_rate;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL, *tmpWxP = NULL, *matrix_data_W_probh1 = NULL, *matrix_probvn_W_probhn = NULL, *matrix_tmp_probh1;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1 = NULL, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL, *tmp_sum2 = NULL, *tmp_sum4 = NULL;
    gsl_rng *r;

    /* Gaussian */
    gsl_vector *p1 = NULL, *pf = NULL, *pf2 = NULL, *p2 = NULL, *invfstdInc = NULL, *std_rate = NULL, *invfstd = NULL;
    p1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    p2 = gsl_vector_calloc(m->n_visible_layer_neurons);
    pf = gsl_vector_calloc(m->n_visible_layer_neurons);
    pf2 = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmp_sum2 = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmp_sum4 = gsl_vector_calloc(m->n_visible_layer_neurons);

    invfstd = gsl_vector_calloc(m->n_visible_layer_neurons);
    gsl_vector_set_zero(invfstd);

    invfstdInc = gsl_vector_calloc(m->n_visible_layer_neurons);
    gsl_vector_set_zero(invfstdInc);

    std_rate = gsl_vector_calloc(n_epochs);
    gsl_vector_set_zero(std_rate);

    matrix_data_W_probh1 = gsl_matrix_calloc(batch_size, m->n_visible_layer_neurons);
    gsl_matrix_set_zero(matrix_data_W_probh1);

    matrix_probvn_W_probhn = gsl_matrix_calloc(batch_size, m->n_visible_layer_neurons);
    gsl_matrix_set_zero(matrix_probvn_W_probhn);

    matrix_tmp_probh1 = gsl_matrix_calloc(batch_size, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(matrix_tmp_probh1);

    gsl_matrix *matrixData = NULL;
    matrixData = gsl_matrix_calloc(batch_size, m->n_visible_layer_neurons);
    gsl_matrix_set_zero(matrixData);

    gsl_vector *tmpVectorz = NULL;
    tmpVectorz = gsl_vector_calloc(m->n_hidden_layer_neurons);

    gsl_vector *vtmp = NULL;
    vtmp = gsl_vector_calloc(m->n_visible_layer_neurons);

    double rr = 0.001;
    float tmp;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpWxP = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    gsl_matrix *tmpMatriz = NULL;
    tmpMatriz = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpMatriz);

    v_std_rate = 30;
    if ((n_epochs / 2) < v_std_rate)
        v_std_rate = n_epochs / 2;

    for (i = 0; i < n_epochs; i++)
    {
        gsl_vector_set(std_rate, i, 0.001);

        if (i < v_std_rate)
        {
            gsl_vector_set(std_rate, i, 0.0);
        }
    }

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);
        errorsum = plsum = 0.0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            line = 0;
            error = pl = 0.0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);
            gsl_vector_set_zero(pf);
            gsl_vector_set_zero(pf2);
            gsl_vector_set_zero(tmp_sum2);
            gsl_vector_set_zero(tmp_sum4);

            for (t = 0; t < batch_size; t++)
            {
                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);
                    for (j = 0; j < m->n_visible_layer_neurons; j++)
                        gsl_matrix_set(matrixData, line, j, gsl_vector_get(m->v, j));

                    /* It accumulates v1 */
                    for (i = 0; i < m->n_visible_layer_neurons; i++)
                    {
                        tmp = gsl_vector_get(m->v, i) / (gsl_vector_get(m->sigma, i) * gsl_vector_get(m->sigma, i));
                        gsl_vector_set(vtmp, i, tmp);
                    }
                    gsl_vector_add(v1, vtmp);

                    /* It computes the P(h=1|v1), i.e., it computes h1 */
                    tmp_probh1 = getProbabilityTurningOnHiddenUnit4Gaussian(m, m->v, m->sigma);
                    for (j = 0; j < m->n_hidden_layer_neurons; j++)
                    {
                        sample = gsl_rng_uniform(r);
                        if (gsl_vector_get(tmp_probh1, j) > sample)
                            gsl_vector_set(m->h, j, 1.0);
                        else
                            gsl_vector_set(m->h, j, 0.0);
                    }

                    for (j = 0; j < m->n_hidden_layer_neurons; j++)
                    {
                        gsl_matrix_set(matrix_tmp_probh1, line, j, gsl_vector_get(tmp_probh1, j));
                    }

                    gsl_vector_memcpy(probh1, tmp_probh1);
                    gsl_vector_add(ctr_probh1, probh1);
                    gsl_vector_free(tmp_probh1);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {
                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        tmp_probvn = getProbabilityTurningOnVisibleUnit4Gaussian(m, m->h, m->sigma);
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            tmp = gsl_vector_get(tmp_probvn, j) + gsl_ran_gaussian(r, gsl_vector_get(m->sigma, j));
                            gsl_vector_set(m->v, j, tmp);
                        }
                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit4Gaussian(m, m->v, m->sigma);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) > sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }

                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, m->v);
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    for (i = 0; i < m->n_visible_layer_neurons; i++)
                    {
                        tmp = gsl_vector_get(m->v, i) / (gsl_vector_get(m->sigma, i) * gsl_vector_get(m->sigma, i));
                        gsl_vector_set(vtmp, i, tmp);
                    }
                    gsl_vector_add(vn, vtmp);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, (gsl_vector_get(D->sample[z].feature, i) / gsl_vector_get(m->sigma, i)) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, (gsl_vector_get(m->v, i) / gsl_vector_get(m->sigma, i)) * gsl_vector_get(probhn, j));
                        }
                    }
                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    /* Starting gaussians */
                    for (j = 0; j < m->n_visible_layer_neurons; j++)
                    {
                        tmp = ((2 * gsl_vector_get(D->sample[z].feature, j)) * ((gsl_vector_get(m->a, j) - (gsl_vector_get(D->sample[z].feature, j) / 2)) / gsl_vector_get(m->sigma, j)));
                        gsl_vector_set(p1, j, tmp);
                    }
                    gsl_vector_add(pf, p1);

                    w = 0;
                    for (i = 0; i < tmpWxP->size1; i++)
                    { /* Visible 3 */
                        tmp = 0.0;
                        for (j = 0; j < tmpWxP->size2; j++)
                        { /* Hidden 4 */
                            tmp += gsl_matrix_get(m->W, i, j) * gsl_vector_get(probh1, j);
                        }
                        gsl_matrix_set(matrix_data_W_probh1, line, i, (gsl_vector_get(D->sample[z].feature, w) * tmp));
                        w++;
                    }

                    for (j = 0; j < m->n_visible_layer_neurons; j++)
                    {
                        tmp = ((2 * gsl_vector_get(probvn, j)) * ((gsl_vector_get(m->a, j) - (gsl_vector_get(probvn, j) / 2)) / gsl_vector_get(m->sigma, j)));
                        gsl_vector_set(p2, j, tmp);
                    }
                    gsl_vector_add(pf2, p2);

                    w = 0;
                    for (i = 0; i < tmpWxP->size1; i++)
                    { /* Visible 3 */
                        tmp = 0.0;
                        for (j = 0; j < tmpWxP->size2; j++)
                        { /* Hidden 4 */
                            tmp += gsl_matrix_get(m->W, i, j) * gsl_vector_get(probhn, j);
                        }
                        gsl_matrix_set(matrix_probvn_W_probhn, line, i, (gsl_vector_get(D->sample[z].feature, w) * tmp));
                        w++;
                    }

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    line++;
                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            if (e > 5)
            {
                m->alpha = 0.9;
            }
            else
            {
                m->alpha = 0.5;
            }

            for (j = 0; j < m->n_visible_layer_neurons; j++)
            {
                tmp = 0.0;
                for (i = 0; i < batch_size; i++)
                {
                    tmp += gsl_matrix_get(matrix_data_W_probh1, i, j);
                }
                gsl_vector_set(tmp_sum2, j, tmp);
            }
            gsl_vector_add(pf, tmp_sum2);

            for (j = 0; j < m->n_visible_layer_neurons; j++)
            {
                tmp = 0.0;
                for (i = 0; i < batch_size; i++)
                {
                    tmp += gsl_matrix_get(matrix_probvn_W_probhn, i, j);
                }
                gsl_vector_set(tmp_sum4, j, tmp);
            }
            gsl_vector_add(pf2, tmp_sum4);
            gsl_vector_sub(pf, pf2);

            /* Updating sigma */
            for (j = 0; j < m->n_visible_layer_neurons; j++)
            {
                tmp = (m->alpha * gsl_vector_get(invfstdInc, j)) + (gsl_vector_get(std_rate, e - 1) / batch_size) * gsl_vector_get(pf, j);
                gsl_vector_set(invfstdInc, j, tmp);
            }

            /* Updating w parameter */
            gsl_matrix_sub(CDpos, CDneg);
            gsl_matrix_scale(CDpos, (rr / batch_size));
            gsl_matrix_scale(tmpW, m->alpha);
            gsl_matrix_add(tmpW, CDpos);
            gsl_matrix_memcpy(auxW, m->W);
            gsl_matrix_scale(auxW, (rr * m->lambda));
            gsl_matrix_sub(tmpW, auxW);

            /* Updating a parameter */
            gsl_vector_sub(v1, vn);
            gsl_vector_scale(v1, (rr / batch_size));
            gsl_vector_scale(tmpa, m->alpha);
            gsl_vector_add(tmpa, v1);

            /* Updating b parameter */
            gsl_vector_sub(ctr_probh1, ctr_probhn);
            gsl_vector_scale(ctr_probh1, (rr / batch_size));
            gsl_vector_scale(tmpb, m->alpha);
            gsl_vector_add(tmpb, ctr_probh1);

            for (j = 0; j < m->n_visible_layer_neurons; j++)
            {
                gsl_vector_set(invfstd, j, (1.0 / gsl_vector_get(m->sigma, j)));
            }
            gsl_vector_add(invfstd, invfstdInc);
            for (j = 0; j < m->n_visible_layer_neurons; j++)
            {
                gsl_vector_set(m->sigma, j, (1.0 / gsl_vector_get(invfstd, j)));
                if (0.005 > gsl_vector_get(m->sigma, j))
                {
                    gsl_vector_set(m->sigma, j, 0.005);
                }
            }
            gsl_matrix_add(m->W, tmpW);
            gsl_vector_add(m->a, tmpa);
            gsl_vector_add(m->b, tmpb);
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "  -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(tmpVectorz);
    gsl_matrix_free(matrixData);
    gsl_matrix_free(tmpMatriz);
    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);
    gsl_vector_free(invfstdInc);
    gsl_vector_free(std_rate);
    gsl_vector_free(invfstd);
    gsl_matrix_free(matrix_data_W_probh1);
    gsl_matrix_free(matrix_probvn_W_probhn);
    gsl_vector_free(tmp_sum2);
    gsl_vector_free(tmp_sum4);
    gsl_vector_free(vtmp);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpWxP);
    gsl_matrix_free(matrix_tmp_probh1);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);

    gsl_vector_free(p1);
    gsl_vector_free(pf);
    gsl_vector_free(p2);
    gsl_vector_free(pf2);

    return error;
}

/* It trains a Gaussian-Bernoulli RBM with Dropout by Constrative Divergence for image reconstruction (binary images)
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size, p]
D: dataset
m: RBM
n_epochs: number of training epochs
n_CD_iterations: number of CD iterations
batch_size: size of batch data
p: hidden neurons dropout rate */
double GaussianBernoulliRBMTrainingbyContrastiveDivergencewithDropout(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size, double p)
{
    int i, j, z, n, t, e, n_batches = ceil((float)D->size / batch_size), ctr, w, line;
    double error, sample, errorsum, pl, plsum, v_std_rate;
    const gsl_rng_type *T;
    gsl_matrix *CDpos = NULL, *CDneg = NULL, *tmpCDpos = NULL, *tmpCDneg = NULL, *tmpW = NULL, *auxW = NULL, *tmpWxP = NULL, *matrix_data_W_probh1 = NULL, *matrix_probvn_W_probhn = NULL, *matrix_tmp_probh1;
    gsl_vector *v1 = NULL, *vn = NULL, *tmpa = NULL, *tmpb = NULL;
    gsl_vector *probh1 = NULL, *probhn = NULL, *probvn = NULL, *ctr_probh1 = NULL, *ctr_probhn = NULL, *tmp_probh1 = NULL, *tmp_probhn = NULL;
    gsl_vector *tmp_probvn = NULL, *tmp_sum2 = NULL, *tmp_sum4 = NULL;
    gsl_rng *r;

    /* Gaussian */
    gsl_vector *p1 = NULL, *pf = NULL, *pf2 = NULL, *p2 = NULL, *invfstdInc = NULL, *std_rate = NULL, *invfstd = NULL;
    p1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    p2 = gsl_vector_calloc(m->n_visible_layer_neurons);
    pf = gsl_vector_calloc(m->n_visible_layer_neurons);
    pf2 = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmp_sum2 = gsl_vector_calloc(m->n_visible_layer_neurons);
    tmp_sum4 = gsl_vector_calloc(m->n_visible_layer_neurons);

    invfstd = gsl_vector_calloc(m->n_visible_layer_neurons);
    gsl_vector_set_zero(invfstd);

    invfstdInc = gsl_vector_calloc(m->n_visible_layer_neurons);
    gsl_vector_set_zero(invfstdInc);

    std_rate = gsl_vector_calloc(n_epochs);
    gsl_vector_set_zero(std_rate);

    matrix_data_W_probh1 = gsl_matrix_calloc(batch_size, m->n_visible_layer_neurons);
    gsl_matrix_set_zero(matrix_data_W_probh1);

    matrix_probvn_W_probhn = gsl_matrix_calloc(batch_size, m->n_visible_layer_neurons);
    gsl_matrix_set_zero(matrix_probvn_W_probhn);

    matrix_tmp_probh1 = gsl_matrix_calloc(batch_size, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(matrix_tmp_probh1);

    gsl_matrix *matrixData = NULL;
    matrixData = gsl_matrix_calloc(batch_size, m->n_visible_layer_neurons);
    gsl_matrix_set_zero(matrixData);

    gsl_vector *tmpVectorz = NULL;
    tmpVectorz = gsl_vector_calloc(m->n_hidden_layer_neurons);

    gsl_vector *vtmp = NULL;
    vtmp = gsl_vector_calloc(m->n_visible_layer_neurons);

    double rr = 0.001;
    float tmp;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v1 = gsl_vector_calloc(m->n_visible_layer_neurons);
    vn = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpa = gsl_vector_calloc(m->n_visible_layer_neurons);

    tmpb = gsl_vector_calloc(m->n_hidden_layer_neurons);
    gsl_vector_set_zero(tmpa);
    gsl_vector_set_zero(tmpb);

    ctr_probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    ctr_probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);

    CDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    CDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDpos = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpCDneg = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpWxP = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    auxW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpW);
    gsl_matrix_set_zero(auxW);

    gsl_matrix *tmpMatriz = NULL;
    tmpMatriz = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    gsl_matrix_set_zero(tmpMatriz);

    v_std_rate = 30;
    if ((n_epochs / 2) < v_std_rate)
        v_std_rate = n_epochs / 2;

    for (i = 0; i < n_epochs; i++)
    {
        gsl_vector_set(std_rate, i, 0.001);

        if (i < v_std_rate)
        {
            gsl_vector_set(std_rate, i, 0.0);
        }
    }

    error = 0;

    /* For each epoch */
    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);
        errorsum = plsum = 0.0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            line = 0;
            error = pl = 0.0;
            gsl_matrix_set_zero(CDpos);
            gsl_matrix_set_zero(CDneg);
            gsl_vector_set_zero(v1);
            gsl_vector_set_zero(vn);
            gsl_vector_set_zero(ctr_probh1);
            gsl_vector_set_zero(ctr_probhn);
            gsl_vector_set_zero(pf);
            gsl_vector_set_zero(pf2);
            gsl_vector_set_zero(tmp_sum2);
            gsl_vector_set_zero(tmp_sum4);

            for (t = 0; t < batch_size; t++)
            {
                /* It computes r for dropping out hidden units */
                InitializeBias4DropoutHiddenUnits(m, p);

                if (z < D->size)
                {
                    ctr++;
                    probh1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probhn = gsl_vector_calloc(m->n_hidden_layer_neurons);
                    probvn = gsl_vector_calloc(m->n_visible_layer_neurons);

                    /* It sets v1 */
                    setVisibleLayer(m, D->sample[z].feature);
                    for (j = 0; j < m->n_visible_layer_neurons; j++)
                        gsl_matrix_set(matrixData, line, j, gsl_vector_get(m->v, j));

                    /* It accumulates v1 */
                    for (i = 0; i < m->n_visible_layer_neurons; i++)
                    {
                        tmp = gsl_vector_get(m->v, i) / (gsl_vector_get(m->sigma, i) * gsl_vector_get(m->sigma, i));
                        gsl_vector_set(vtmp, i, tmp);
                    }
                    gsl_vector_add(v1, vtmp);

                    /* It computes the P(h=1|v1), i.e., it computes h1 */
                    tmp_probh1 = getProbabilityTurningOnHiddenUnit4Gaussian4Dropout(m, m->r, m->v, m->sigma);
                    for (j = 0; j < m->n_hidden_layer_neurons; j++)
                    {
                        sample = gsl_rng_uniform(r);
                        if (gsl_vector_get(tmp_probh1, j) > sample)
                            gsl_vector_set(m->h, j, 1.0);
                        else
                            gsl_vector_set(m->h, j, 0.0);
                    }

                    for (j = 0; j < m->n_hidden_layer_neurons; j++)
                    {
                        gsl_matrix_set(matrix_tmp_probh1, line, j, gsl_vector_get(tmp_probh1, j));
                    }

                    gsl_vector_memcpy(probh1, tmp_probh1);
                    gsl_vector_add(ctr_probh1, probh1);
                    gsl_vector_free(tmp_probh1);

                    /* For each CD iteration */
                    for (i = 1; i <= n_CD_iterations; i++)
                    {
                        /* It computes the P(v2=1|h1), i.e., it computes v2 */
                        tmp_probvn = getProbabilityTurningOnVisibleUnit4Gaussian4Dropout(m, m->r, m->h, m->sigma);
                        for (j = 0; j < m->n_visible_layer_neurons; j++)
                        {
                            tmp = gsl_vector_get(tmp_probvn, j) + gsl_ran_gaussian(r, gsl_vector_get(m->sigma, j));
                            gsl_vector_set(m->v, j, tmp);
                        }
                        /* It computes the P(h2=1|v2), i.e., it computes h2 (hn) */
                        tmp_probhn = getProbabilityTurningOnHiddenUnit4Gaussian4Dropout(m, m->r, m->v, m->sigma);
                        for (j = 0; j < m->n_hidden_layer_neurons; j++)
                        {
                            sample = gsl_rng_uniform(r);
                            if (gsl_vector_get(tmp_probhn, j) > sample)
                                gsl_vector_set(m->h, j, 1.0);
                            else
                                gsl_vector_set(m->h, j, 0.0);
                        }

                        if (i == n_CD_iterations)
                        { /* In case of n_CD_iterations > 1 */
                            gsl_vector_memcpy(probhn, tmp_probhn);
                            gsl_vector_add(ctr_probhn, probhn);
                            gsl_vector_memcpy(probvn, m->v);
                        }

                        gsl_vector_free(tmp_probhn);
                        gsl_vector_free(tmp_probvn);
                    }

                    /* It accumulates vn */
                    for (i = 0; i < m->n_visible_layer_neurons; i++)
                    {
                        tmp = gsl_vector_get(m->v, i) / (gsl_vector_get(m->sigma, i) * gsl_vector_get(m->sigma, i));
                        gsl_vector_set(vtmp, i, tmp);
                    }
                    gsl_vector_add(vn, vtmp);

                    for (i = 0; i < tmpCDpos->size1; i++)
                    {
                        for (j = 0; j < tmpCDpos->size2; j++)
                        {
                            gsl_matrix_set(tmpCDpos, i, j, (gsl_vector_get(D->sample[z].feature, i) / gsl_vector_get(m->sigma, i)) * gsl_vector_get(probh1, j));
                            gsl_matrix_set(tmpCDneg, i, j, (gsl_vector_get(m->v, i) / gsl_vector_get(m->sigma, i)) * gsl_vector_get(probhn, j));
                        }
                    }
                    gsl_matrix_add(CDpos, tmpCDpos);
                    gsl_matrix_add(CDneg, tmpCDneg);

                    /* Starting gaussians */
                    for (j = 0; j < m->n_visible_layer_neurons; j++)
                    {
                        tmp = ((2 * gsl_vector_get(D->sample[z].feature, j)) * ((gsl_vector_get(m->a, j) - (gsl_vector_get(D->sample[z].feature, j) / 2)) / gsl_vector_get(m->sigma, j)));
                        gsl_vector_set(p1, j, tmp);
                    }
                    gsl_vector_add(pf, p1);

                    w = 0;
                    for (i = 0; i < tmpWxP->size1; i++)
                    { /* Visible 3 */
                        tmp = 0.0;
                        for (j = 0; j < tmpWxP->size2; j++)
                        { /* Hidden 4 */
                            tmp += gsl_matrix_get(m->W, i, j) * gsl_vector_get(probh1, j);
                        }
                        gsl_matrix_set(matrix_data_W_probh1, line, i, (gsl_vector_get(D->sample[z].feature, w) * tmp));
                        w++;
                    }

                    for (j = 0; j < m->n_visible_layer_neurons; j++)
                    {
                        tmp = ((2 * gsl_vector_get(probvn, j)) * ((gsl_vector_get(m->a, j) - (gsl_vector_get(probvn, j) / 2)) / gsl_vector_get(m->sigma, j)));
                        gsl_vector_set(p2, j, tmp);
                    }
                    gsl_vector_add(pf2, p2);

                    w = 0;
                    for (i = 0; i < tmpWxP->size1; i++)
                    { /* Visible 3 */
                        tmp = 0.0;
                        for (j = 0; j < tmpWxP->size2; j++)
                        { /* Hidden 4 */
                            tmp += gsl_matrix_get(m->W, i, j) * gsl_vector_get(probhn, j);
                        }
                        gsl_matrix_set(matrix_probvn_W_probhn, line, i, (gsl_vector_get(D->sample[z].feature, w) * tmp));
                        w++;
                    }

                    error += getReconstructionError(D->sample[z].feature, probvn);
                    pl += getPseudoLikelihood(m, m->v);

                    gsl_vector_free(probh1);
                    gsl_vector_free(probhn);
                    gsl_vector_free(probvn);

                    line++;
                    z++;
                }
            }

            errorsum = errorsum + error / ctr;
            plsum = plsum + pl / ctr;

            if (e > 5)
            {
                m->alpha = 0.9;
            }
            else
            {
                m->alpha = 0.5;
            }

            for (j = 0; j < m->n_visible_layer_neurons; j++)
            {
                tmp = 0.0;
                for (i = 0; i < batch_size; i++)
                {
                    tmp += gsl_matrix_get(matrix_data_W_probh1, i, j);
                }
                gsl_vector_set(tmp_sum2, j, tmp);
            }
            gsl_vector_add(pf, tmp_sum2);

            for (j = 0; j < m->n_visible_layer_neurons; j++)
            {
                tmp = 0.0;
                for (i = 0; i < batch_size; i++)
                {
                    tmp += gsl_matrix_get(matrix_probvn_W_probhn, i, j);
                }
                gsl_vector_set(tmp_sum4, j, tmp);
            }
            gsl_vector_add(pf2, tmp_sum4);
            gsl_vector_sub(pf, pf2);

            /* Updating sigma */
            for (j = 0; j < m->n_visible_layer_neurons; j++)
            {
                tmp = (m->alpha * gsl_vector_get(invfstdInc, j)) + (gsl_vector_get(std_rate, e - 1) / batch_size) * gsl_vector_get(pf, j);
                gsl_vector_set(invfstdInc, j, tmp);
            }

            /* Updating w parameter */
            gsl_matrix_sub(CDpos, CDneg);
            gsl_matrix_scale(CDpos, (rr / batch_size));
            gsl_matrix_scale(tmpW, m->alpha);
            gsl_matrix_add(tmpW, CDpos);
            gsl_matrix_memcpy(auxW, m->W);
            gsl_matrix_scale(auxW, (rr * m->lambda));
            gsl_matrix_sub(tmpW, auxW);

            /* Updating a parameter */
            gsl_vector_sub(v1, vn);
            gsl_vector_scale(v1, (rr / batch_size));
            gsl_vector_scale(tmpa, m->alpha);
            gsl_vector_add(tmpa, v1);

            /* Updating b parameter */
            gsl_vector_sub(ctr_probh1, ctr_probhn);
            gsl_vector_scale(ctr_probh1, (rr / batch_size));
            gsl_vector_scale(tmpb, m->alpha);
            gsl_vector_add(tmpb, ctr_probh1);

            for (j = 0; j < m->n_visible_layer_neurons; j++)
            {
                gsl_vector_set(invfstd, j, (1.0 / gsl_vector_get(m->sigma, j)));
            }
            gsl_vector_add(invfstd, invfstdInc);
            for (j = 0; j < m->n_visible_layer_neurons; j++)
            {
                gsl_vector_set(m->sigma, j, (1.0 / gsl_vector_get(invfstd, j)));
                if (0.005 > gsl_vector_get(m->sigma, j))
                {
                    gsl_vector_set(m->sigma, j, 0.005);
                }
            }
            gsl_matrix_add(m->W, tmpW);
            gsl_vector_add(m->a, tmpa);
            gsl_vector_add(m->b, tmpb);
        }

        error = errorsum / n_batches;
        pl = plsum / n_batches;
        fprintf(stderr, "  -> Reconstruction error: %lf with pseudo-likelihood of %lf", error, pl);
        fprintf(stdout, "%d %lf %lf\n", e, error, pl);

        if (error < 0.0001)
            e = n_epochs + 1;
    }

    gsl_rng_free(r);

    gsl_vector_free(tmpVectorz);
    gsl_matrix_free(matrixData);
    gsl_matrix_free(tmpMatriz);
    gsl_vector_free(v1);
    gsl_vector_free(vn);
    gsl_vector_free(tmpa);
    gsl_vector_free(tmpb);
    gsl_vector_free(ctr_probh1);
    gsl_vector_free(ctr_probhn);
    gsl_vector_free(invfstdInc);
    gsl_vector_free(std_rate);
    gsl_vector_free(invfstd);
    gsl_matrix_free(matrix_data_W_probh1);
    gsl_matrix_free(matrix_probvn_W_probhn);
    gsl_vector_free(tmp_sum2);
    gsl_vector_free(tmp_sum4);
    gsl_vector_free(vtmp);

    gsl_matrix_free(CDpos);
    gsl_matrix_free(CDneg);
    gsl_matrix_free(tmpCDneg);
    gsl_matrix_free(tmpWxP);
    gsl_matrix_free(matrix_tmp_probh1);
    gsl_matrix_free(tmpCDpos);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(auxW);

    gsl_vector_free(p1);
    gsl_vector_free(pf);
    gsl_vector_free(p2);
    gsl_vector_free(pf2);

    return error;
}

/* It trains a Discriminative Gaussian-Bernoulli RBM by Constrative Divergence for pattern classification
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size]
D: dataset
m: DRBM
n_epocs: number of epochs
n_CD_iterations: number of Constrastive Divergence iterations
batch_size: size of the mini-batch */
double DiscriminativeGaussianBernoulliRBMTrainingbyContrastiveDivergence(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size)
{
    int e, z, j, i, n, n_batches = ceil((float)D->size / batch_size), t, ctr;
    gsl_vector *y0 = NULL, *y1 = NULL, *py1 = NULL, *ph0 = NULL, *ph1 = NULL, *pv1 = NULL, *acc_v0 = NULL, *acc_v1 = NULL;
    gsl_vector *acc_h0 = NULL, *acc_h1 = NULL, *acc_y0 = NULL, *acc_y1 = NULL, *delta_a = NULL, *delta_b = NULL, *delta_c = NULL;
    gsl_matrix *_posW = NULL, *_negW = NULL, *posW = NULL, *negW = NULL, *_posU = NULL, *_negU = NULL, *posU = NULL, *negU = NULL;
    gsl_matrix *tmpW = NULL, *tmpU = NULL, *delta_W = NULL, *delta_U = NULL;
    double sample, error, errorsum, train_error;
    const gsl_rng_type *T = NULL;
    gsl_rng *r = NULL;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    _posW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    posW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    _negW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    negW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    _posU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);
    posU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);
    _negU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);
    negU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);

    delta_W = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    delta_U = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);

    acc_v0 = gsl_vector_calloc(m->n_visible_layer_neurons);
    acc_v1 = gsl_vector_calloc(m->n_visible_layer_neurons);

    acc_h0 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    acc_h1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    acc_y0 = gsl_vector_calloc(m->n_labels);
    acc_y1 = gsl_vector_calloc(m->n_labels);

    delta_a = gsl_vector_calloc(m->n_visible_layer_neurons);
    delta_b = gsl_vector_calloc(m->n_hidden_layer_neurons);
    delta_c = gsl_vector_calloc(m->n_labels);

    train_error = 0;

    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = 0.0;
            gsl_matrix_set_zero(posW);
            gsl_matrix_set_zero(negW);
            gsl_matrix_set_zero(posU);
            gsl_matrix_set_zero(negU);
            gsl_vector_set_zero(acc_v0);
            gsl_vector_set_zero(acc_v1);
            gsl_vector_set_zero(acc_h0);
            gsl_vector_set_zero(acc_h1);
            gsl_vector_set_zero(acc_y0);
            gsl_vector_set_zero(acc_y1);

            for (t = 0; t < batch_size; t++)
            {
                if (z < D->size)
                {
                    ctr++;
                    setVisibleLayer(m, D->sample[z].feature);
                    gsl_vector_add(acc_v0, m->v);
                    y0 = label2binary_gsl_vector(D->sample[z].label, D->nlabels);
                    gsl_vector_add(acc_y0, y0);

                    /* It computes the P(h0|v0,y0) */
                    ph0 = getDiscriminativeProbabilityTurningOnHiddenUnit4GaussianVisibleUnit(m, y0);
                    for (j = 0; j < m->n_hidden_layer_neurons; j++)
                    {
                        sample = gsl_rng_uniform(r);
                        if (gsl_vector_get(ph0, j) > sample)
                            gsl_vector_set(m->h, j, 1.0);
                        else
                            gsl_vector_set(m->h, j, 0.0);
                    }
                    gsl_vector_add(acc_h0, ph0);

                    /* It computes the P(v1|h0) */
                    pv1 = getDiscriminativeProbabilityTurningOnVisibleUnit4GaussianVisibleUnit(m, m->h);
                    for (j = 0; j < m->n_visible_layer_neurons; j++)
                    {
                        sample = gsl_rng_uniform(r);
                        if (gsl_vector_get(pv1, j) > sample)
                            gsl_vector_set(m->v, j, 1.0);
                        else
                            gsl_vector_set(m->v, j, 0.0);
                    }
                    gsl_vector_add(acc_v1, m->v);

                    /* It computes the P(y1|h0) */
                    py1 = getDiscriminativeProbabilityLabelUnit(m);
                    y1 = gsl_vector_calloc(py1->size);
                    gsl_vector_set(y1, gsl_vector_max_index(py1), 1.0); /* It samples the class with highest probability */
                    gsl_vector_add(acc_y1, y1);

                    /* It computes the P(h1|y1,v1) */
                    ph1 = getDiscriminativeProbabilityTurningOnHiddenUnit(m, y1);
                    for (j = 0; j < m->n_hidden_layer_neurons; j++)
                    {
                        sample = gsl_rng_uniform(r);
                        if (gsl_vector_get(ph1, j) > sample)
                            gsl_vector_set(m->h, j, 1.0);
                        else
                            gsl_vector_set(m->h, j, 0.0);
                    }
                    gsl_vector_add(acc_h1, ph1);

                    for (i = 0; i < _posW->size1; i++)
                    {
                        for (j = 0; j < _posW->size2; j++)
                        {
                            gsl_matrix_set(_posW, i, j, gsl_vector_get(ph0, j) * (gsl_vector_get(D->sample[z].feature, i) / gsl_vector_get(m->sigma, i)));
                            gsl_matrix_set(_negW, i, j, gsl_vector_get(ph1, j) * (gsl_vector_get(m->v, i) / gsl_vector_get(m->sigma, i)));
                        }
                    }

                    for (i = 0; i < _posU->size1; i++)
                    {
                        for (j = 0; j < _posU->size2; j++)
                        {
                            gsl_matrix_set(_posU, i, j, gsl_vector_get(ph0, j) * gsl_vector_get(y0, i));
                            gsl_matrix_set(_negU, i, j, gsl_vector_get(ph1, j) * gsl_vector_get(y1, i));
                        }
                    }

                    gsl_matrix_add(posW, _posW);
                    gsl_matrix_add(negW, _negW);

                    gsl_matrix_add(posU, _posU);
                    gsl_matrix_add(negU, _negU);

                    error += getReconstructionError(y0, py1);

                    gsl_vector_free(y0);
                    gsl_vector_free(y1);
                    gsl_vector_free(py1);
                    gsl_vector_free(ph0);
                    gsl_vector_free(ph1);
                    gsl_vector_free(pv1);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;

            /* Updating W parameter */
            gsl_matrix_sub(posW, negW); /* posW = posW-negW */
            gsl_matrix_scale(posW, 1.0 / ctr);
            gsl_matrix_scale(posW, m->eta); /* posW = eta*posW */
            gsl_matrix_memcpy(tmpW, m->W);
            gsl_matrix_scale(tmpW, -m->lambda);  /* tmp = -lambda*W */
            gsl_matrix_add(posW, tmpW);          /* posW = eta*(posW-negW) - lambda*W */
            gsl_matrix_scale(delta_W, m->alpha); /* delta_W = alpha*delta_W */
            gsl_matrix_add(delta_W, posW);       /* delta_W = eta*(posW-negW) - lambda*W * alpha*delta_W */
            gsl_matrix_add(m->W, delta_W);       /* W = W + delta_W */

            /* Updating U parameter */
            gsl_matrix_sub(posU, negU); /* posU = posU-negU */
            gsl_matrix_scale(posU, 1.0 / ctr);
            gsl_matrix_scale(posU, m->eta); /* posU = eta*posU */
            gsl_matrix_memcpy(tmpU, m->U);
            gsl_matrix_scale(tmpU, -m->lambda);  /* tmp = -lambda*U */
            gsl_matrix_add(posU, tmpU);          /* posU = eta*(posU-negU) -lambda*U */
            gsl_matrix_scale(delta_U, m->alpha); /* delta_U = alpha*delta_U */
            gsl_matrix_add(delta_U, posU);       /* delta_U = eta*(posU-negU) -lambda*U + alpha*delta_U */
            gsl_matrix_add(m->U, delta_U);       /* U = U + delta_U */

            /* Updating a parameter */
            gsl_vector_div(acc_v0, m->sigma); /* v0 = v0/sigma */
            gsl_vector_div(acc_v1, m->sigma); /* v1 = v1/sigma */
            gsl_vector_sub(acc_v0, acc_v1);   /* v0 = v0 - v1 */
            gsl_vector_scale(acc_v0, 1.0 / ctr);
            gsl_vector_scale(acc_v0, m->eta);    /* v0 = eta*v0 */
            gsl_vector_scale(delta_a, m->alpha); /* delta_a = alpha*delta_a */
            gsl_vector_add(delta_a, acc_v0);     /* delta_a = eta*(v0-v1) + alpha*delta_a */
            gsl_vector_add(m->a, delta_a);       /* a = a + delta_a */

            /* Updating b parameter */
            gsl_vector_sub(acc_h0, acc_h1); /* h0 = h0 - h1 */
            gsl_vector_scale(acc_h0, 1.0 / ctr);
            gsl_vector_scale(acc_h0, m->eta);    /* h0 = eta*h0 */
            gsl_vector_scale(delta_b, m->alpha); /* delta_b = alpha*delta_b */
            gsl_vector_add(delta_b, acc_h0);     /* delta_b = eta*(h0 - h1) + alpha*delta_b */
            gsl_vector_add(m->b, delta_b);       /* b = b + delta_b */

            /* Updating c parameter */
            gsl_vector_sub(acc_y0, acc_y1); /* y0 = y0 - y1 */
            gsl_vector_scale(acc_y0, 1.0 / ctr);
            gsl_vector_scale(acc_y0, m->eta);    /* y0 = eta*y0 */
            gsl_vector_scale(delta_c, m->alpha); /* delta_c = alpha*delta_c */
            gsl_vector_add(delta_c, acc_y0);     /* delta_c = eta*(y0 - y1) + alpha*delta_c */
            gsl_vector_add(m->c, delta_c);       /* c = c + delta_c */
        }

        train_error = errorsum / n_batches;
        fprintf(stderr, "MSE classification error: %lf OK", train_error);
    }

    gsl_rng_free(r);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(tmpU);
    gsl_matrix_free(_posW);
    gsl_matrix_free(posW);
    gsl_matrix_free(_negW);
    gsl_matrix_free(negW);
    gsl_matrix_free(_posU);
    gsl_matrix_free(_negU);
    gsl_matrix_free(posU);
    gsl_matrix_free(negU);
    gsl_matrix_free(delta_W);
    gsl_matrix_free(delta_U);
    gsl_vector_free(acc_v0);
    gsl_vector_free(acc_v1);
    gsl_vector_free(acc_h0);
    gsl_vector_free(acc_h1);
    gsl_vector_free(acc_y0);
    gsl_vector_free(acc_y1);
    gsl_vector_free(delta_a);
    gsl_vector_free(delta_b);
    gsl_vector_free(delta_c);

    return train_error;
}

/* It trains a Discriminative Gaussian-Bernoulli RBM with Dropout by Constrative Divergence for pattern classification
Parameters: [D, m, n_epochs, n_CD_iterations, batch_size, p]
D: dataset
m: DRBM
n_epocs: number of epochs
n_CD_iterations: number of Constrastive Divergence iterations
batch_size: size of the mini-batch
p: hidden neurons dropout rate */
double DiscriminativeGaussianBernoulliRBMTrainingbyContrastiveDivergencewithDropout(Dataset *D, RBM *m, int n_epochs, int n_CD_iterations, int batch_size, double p)
{
    int e, z, j, i, n, n_batches = ceil((float)D->size / batch_size), t, ctr;
    gsl_vector *y0 = NULL, *y1 = NULL, *py1 = NULL, *ph0 = NULL, *ph1 = NULL, *pv1 = NULL, *acc_v0 = NULL, *acc_v1 = NULL;
    gsl_vector *acc_h0 = NULL, *acc_h1 = NULL, *acc_y0 = NULL, *acc_y1 = NULL, *delta_a = NULL, *delta_b = NULL, *delta_c = NULL;
    gsl_matrix *_posW = NULL, *_negW = NULL, *posW = NULL, *negW = NULL, *_posU = NULL, *_negU = NULL, *posU = NULL, *negU = NULL;
    gsl_matrix *tmpW = NULL, *tmpU = NULL, *delta_W = NULL, *delta_U = NULL;
    double sample, error, errorsum, train_error;
    const gsl_rng_type *T = NULL;
    gsl_rng *r = NULL;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    _posW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    posW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    _negW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    negW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);

    _posU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);
    posU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);
    _negU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);
    negU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);

    tmpW = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    tmpU = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);

    delta_W = gsl_matrix_calloc(m->n_visible_layer_neurons, m->n_hidden_layer_neurons);
    delta_U = gsl_matrix_calloc(m->n_labels, m->n_hidden_layer_neurons);

    acc_v0 = gsl_vector_calloc(m->n_visible_layer_neurons);
    acc_v1 = gsl_vector_calloc(m->n_visible_layer_neurons);

    acc_h0 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    acc_h1 = gsl_vector_calloc(m->n_hidden_layer_neurons);
    acc_y0 = gsl_vector_calloc(m->n_labels);
    acc_y1 = gsl_vector_calloc(m->n_labels);

    delta_a = gsl_vector_calloc(m->n_visible_layer_neurons);
    delta_b = gsl_vector_calloc(m->n_hidden_layer_neurons);
    delta_c = gsl_vector_calloc(m->n_labels);

    train_error = 0;

    for (e = 1; e <= n_epochs; e++)
    {
        fprintf(stderr, "\nRunning epoch %d ... ", e);

        errorsum = 0;
        z = 0;

        /* For each batch */
        for (n = 1; n <= n_batches; n++)
        {
            ctr = 0;
            error = 0.0;
            gsl_matrix_set_zero(posW);
            gsl_matrix_set_zero(negW);
            gsl_matrix_set_zero(posU);
            gsl_matrix_set_zero(negU);
            gsl_vector_set_zero(acc_v0);
            gsl_vector_set_zero(acc_v1);
            gsl_vector_set_zero(acc_h0);
            gsl_vector_set_zero(acc_h1);
            gsl_vector_set_zero(acc_y0);
            gsl_vector_set_zero(acc_y1);

            for (t = 0; t < batch_size; t++)
            {
                /* It computes r for dropping out hidden units */
                InitializeBias4DropoutHiddenUnits(m, p);

                if (z < D->size)
                {
                    ctr++;
                    setVisibleLayer(m, D->sample[z].feature);
                    gsl_vector_add(acc_v0, m->v);
                    y0 = label2binary_gsl_vector(D->sample[z].label, D->nlabels);
                    gsl_vector_add(acc_y0, y0);

                    /* It computes the P(h0|v0,y0) */
                    ph0 = getDiscriminativeProbabilityTurningOnHiddenUnit4GaussianVisibleUnit4Dropout(m, m->r, y0);
                    for (j = 0; j < m->n_hidden_layer_neurons; j++)
                    {
                        sample = gsl_rng_uniform(r);
                        if (gsl_vector_get(ph0, j) > sample)
                            gsl_vector_set(m->h, j, 1.0);
                        else
                            gsl_vector_set(m->h, j, 0.0);
                    }
                    gsl_vector_add(acc_h0, ph0);

                    /* It computes the P(v1|h0) */
                    pv1 = getDiscriminativeProbabilityTurningOnVisibleUnit4GaussianVisibleUnit4Dropout(m, m->r, m->h);
                    for (j = 0; j < m->n_visible_layer_neurons; j++)
                    {
                        sample = gsl_rng_uniform(r);
                        if (gsl_vector_get(pv1, j) > sample)
                            gsl_vector_set(m->v, j, 1.0);
                        else
                            gsl_vector_set(m->v, j, 0.0);
                    }
                    gsl_vector_add(acc_v1, m->v);

                    /* It computes the P(y1|h0) */
                    py1 = getDiscriminativeProbabilityLabelUnit(m);
                    y1 = gsl_vector_calloc(py1->size);
                    gsl_vector_set(y1, gsl_vector_max_index(py1), 1.0); /* It samples the class with highest probability */
                    gsl_vector_add(acc_y1, y1);

                    /* It computes the P(h1|y1,v1) */
                    ph1 = getDiscriminativeProbabilityTurningOnHiddenUnit4Dropout(m, m->r, y1);
                    for (j = 0; j < m->n_hidden_layer_neurons; j++)
                    {
                        sample = gsl_rng_uniform(r);
                        if (gsl_vector_get(ph1, j) > sample)
                            gsl_vector_set(m->h, j, 1.0);
                        else
                            gsl_vector_set(m->h, j, 0.0);
                    }
                    gsl_vector_add(acc_h1, ph1);

                    for (i = 0; i < _posW->size1; i++)
                    {
                        for (j = 0; j < _posW->size2; j++)
                        {
                            gsl_matrix_set(_posW, i, j, gsl_vector_get(ph0, j) * (gsl_vector_get(D->sample[z].feature, i) / gsl_vector_get(m->sigma, i)));
                            gsl_matrix_set(_negW, i, j, gsl_vector_get(ph1, j) * (gsl_vector_get(m->v, i) / gsl_vector_get(m->sigma, i)));
                        }
                    }

                    for (i = 0; i < _posU->size1; i++)
                    {
                        for (j = 0; j < _posU->size2; j++)
                        {
                            gsl_matrix_set(_posU, i, j, gsl_vector_get(ph0, j) * gsl_vector_get(y0, i));
                            gsl_matrix_set(_negU, i, j, gsl_vector_get(ph1, j) * gsl_vector_get(y1, i));
                        }
                    }

                    gsl_matrix_add(posW, _posW);
                    gsl_matrix_add(negW, _negW);

                    gsl_matrix_add(posU, _posU);
                    gsl_matrix_add(negU, _negU);

                    error += getReconstructionError(y0, py1);

                    gsl_vector_free(y0);
                    gsl_vector_free(y1);
                    gsl_vector_free(py1);
                    gsl_vector_free(ph0);
                    gsl_vector_free(ph1);
                    gsl_vector_free(pv1);

                    z++;
                }
            }

            errorsum = errorsum + error / ctr;

            /* Updating W parameter */
            gsl_matrix_sub(posW, negW); /* posW = posW-negW */
            gsl_matrix_scale(posW, 1.0 / ctr);
            gsl_matrix_scale(posW, m->eta); /* posW = eta*posW */
            gsl_matrix_memcpy(tmpW, m->W);
            gsl_matrix_scale(tmpW, -m->lambda);  /* tmp = -lambda*W */
            gsl_matrix_add(posW, tmpW);          /* posW = eta*(posW-negW) - lambda*W */
            gsl_matrix_scale(delta_W, m->alpha); /* delta_W = alpha*delta_W */
            gsl_matrix_add(delta_W, posW);       /* delta_W = eta*(posW-negW) - lambda*W * alpha*delta_W */
            gsl_matrix_add(m->W, delta_W);       /* W = W + delta_W */

            /* Updating U parameter */
            gsl_matrix_sub(posU, negU); /* posU = posU-negU */
            gsl_matrix_scale(posU, 1.0 / ctr);
            gsl_matrix_scale(posU, m->eta); /* posU = eta*posU */
            gsl_matrix_memcpy(tmpU, m->U);
            gsl_matrix_scale(tmpU, -m->lambda);  /* tmp = -lambda*U */
            gsl_matrix_add(posU, tmpU);          /* posU = eta*(posU-negU) -lambda*U */
            gsl_matrix_scale(delta_U, m->alpha); /* delta_U = alpha*delta_U */
            gsl_matrix_add(delta_U, posU);       /* delta_U = eta*(posU-negU) -lambda*U + alpha*delta_U */
            gsl_matrix_add(m->U, delta_U);       /* U = U + delta_U */

            /* Updating a parameter */
            gsl_vector_div(acc_v0, m->sigma); /* v0 = v0/sigma */
            gsl_vector_div(acc_v1, m->sigma); /* v1 = v1/sigma */
            gsl_vector_sub(acc_v0, acc_v1);   /* v0 = v0 - v1 */
            gsl_vector_scale(acc_v0, 1.0 / ctr);
            gsl_vector_scale(acc_v0, m->eta);    /* v0 = eta*v0 */
            gsl_vector_scale(delta_a, m->alpha); /* delta_a = alpha*delta_a */
            gsl_vector_add(delta_a, acc_v0);     /* delta_a = eta*(v0-v1) + alpha*delta_a */
            gsl_vector_add(m->a, delta_a);       /* a = a + delta_a */

            /* Updating b parameter */
            gsl_vector_sub(acc_h0, acc_h1); /* h0 = h0 - h1 */
            gsl_vector_scale(acc_h0, 1.0 / ctr);
            gsl_vector_scale(acc_h0, m->eta);    /* h0 = eta*h0 */
            gsl_vector_scale(delta_b, m->alpha); /* delta_b = alpha*delta_b */
            gsl_vector_add(delta_b, acc_h0);     /* delta_b = eta*(h0 - h1) + alpha*delta_b */
            gsl_vector_add(m->b, delta_b);       /* b = b + delta_b */

            /* Updating c parameter */
            gsl_vector_sub(acc_y0, acc_y1); /* y0 = y0 - y1 */
            gsl_vector_scale(acc_y0, 1.0 / ctr);
            gsl_vector_scale(acc_y0, m->eta);    /* y0 = eta*y0 */
            gsl_vector_scale(delta_c, m->alpha); /* delta_c = alpha*delta_c */
            gsl_vector_add(delta_c, acc_y0);     /* delta_c = eta*(y0 - y1) + alpha*delta_c */
            gsl_vector_add(m->c, delta_c);       /* c = c + delta_c */
        }

        train_error = errorsum / n_batches;
        fprintf(stderr, "MSE classification error: %lf OK", train_error);
    }

    gsl_rng_free(r);
    gsl_matrix_free(tmpW);
    gsl_matrix_free(tmpU);
    gsl_matrix_free(_posW);
    gsl_matrix_free(posW);
    gsl_matrix_free(_negW);
    gsl_matrix_free(negW);
    gsl_matrix_free(_posU);
    gsl_matrix_free(_negU);
    gsl_matrix_free(posU);
    gsl_matrix_free(negU);
    gsl_matrix_free(delta_W);
    gsl_matrix_free(delta_U);
    gsl_vector_free(acc_v0);
    gsl_vector_free(acc_v1);
    gsl_vector_free(acc_h0);
    gsl_vector_free(acc_h1);
    gsl_vector_free(acc_y0);
    gsl_vector_free(acc_y1);
    gsl_vector_free(delta_a);
    gsl_vector_free(delta_b);
    gsl_vector_free(delta_c);

    return train_error;
}
/**************************/

/* Bernoulli RBM reconstruction/classification */

/* It reconstructs an input dataset given a trained RBM
Parameters: [D, m]
D: dataset
m: RBM */
double BernoulliRBMReconstruction(Dataset *D, RBM *m)
{
    double error = 0.0;
    int i;
    gsl_vector *h_prime = NULL, *v_prime = NULL;

    for (i = 0; i < D->size; i++)
    {
        h_prime = getProbabilityTurningOnHiddenUnit(m, D->sample[i].feature);
        v_prime = getProbabilityTurningOnVisibleUnit(m, h_prime);
        error += getReconstructionError(D->sample[i].feature, v_prime);
        gsl_vector_free(h_prime);
        gsl_vector_free(v_prime);
    }
    error /= D->size;

    return error;
}

/* It reconstructs an input dataset given a trained Gaussian RBM
Parameters: [D, m]
D: dataset
m: RBM */
/* It reconstructs an input dataset given a trained RBM */
double GaussianBernoulliRBMReconstruction(Dataset *D, RBM *m)
{
    double error = 0.0;
    int i;
    gsl_vector *h_prime = NULL, *v_prime = NULL;

    for (i = 0; i < D->size; i++)
    {
        h_prime = getProbabilityTurningOnHiddenUnit4Gaussian(m, D->sample[i].feature, m->sigma);
        v_prime = getProbabilityTurningOnVisibleUnit4Gaussian(m, h_prime, m->sigma);
        error += getReconstructionError(D->sample[i].feature, v_prime);
        gsl_vector_free(h_prime);
        gsl_vector_free(v_prime);
    }
    error /= D->size;

    return error;
}

/* It classifies an input dataset given a trained RBM
Parameters: [D, m]
D: dataset
m: RBM */
void _DiscriminativeBernoulliRBMClassification(Dataset *D, RBM *m)
{
    int i, y, label;
    double proby_x, maxproby_x;
    gsl_vector *prob = NULL;

    prob = gsl_vector_alloc(D->nlabels);

    for (i = 0; i < D->size; i++)
    {
        label = 0;
        for (y = 0; y < D->nlabels; y++)
        {
            gsl_vector_set(prob, y, FreeEnergy4DRBM(m, y, D->sample[i].feature));
        }
        maxproby_x = -99999999999;
        for (y = 0; y < D->nlabels; y++)
        {
            proby_x = gsl_vector_get(prob, y);
            if (proby_x > maxproby_x)
            {
                maxproby_x = proby_x;
                label = y + 1;
            }
        }
        D->sample[i].predict = label;
    }
    gsl_vector_free(prob);
}

/* It classifies an input dataset given a trained RBM and it outputs the classification error
This implementation is based on Equation 2 of paper "Learning Algorithms for the Classification Restricted Boltzmann Machine"
Parameters: [D, m]
D: dataset
m: RBM */
double DiscriminativeBernoulliRBMClassification(Dataset *D, RBM *m)
{
    int i, y;
    double maxprob, prob, Acc;
    Subgraph *g = NULL;

    for (i = 0; i < D->size; i++)
    {
        maxprob = -99999999999;
        for (y = 0; y < D->nlabels; y++)
        {
            prob = FreeEnergy4DRBM(m, y, D->sample[i].feature);
            if (prob > maxprob)
            {
                maxprob = prob;
                D->sample[i].predict = y + 1;
            }
        }
    }
    g = Dataset2Subgraph(D);
    Acc = opf_Accuracy(g);
    DestroySubgraph(&g);

    return 1 - Acc;
}
/**************************/

/* Auxiliary functions */

/* It computes the free energy of a given sample
Parameters: [m, v]
m: RBM
v: input sample
The free energy is computed based on http://deeplearning.net/tutorial/rbm.html (Equation 8) */
double FreeEnergy(RBM *m, gsl_vector *v)
{
    int i, j;
    double wv_b, sum = 0, b_v = 0;

    for (i = 0; i < m->n_visible_layer_neurons; i++)
        b_v += (gsl_vector_get(m->a, i) * gsl_vector_get(v, i)); /* It computes a*v */
    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        wv_b = 0;
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            wv_b += (gsl_matrix_get(m->W, i, j) * gsl_vector_get(v, i)); /* It computes the w*v */
        wv_b += gsl_vector_get(m->b, j);                                 /* It computes the w*v+b */
        wv_b = 1 + exp(wv_b);                                            /* It computes 1+exp(wv_b) */
        sum += log(wv_b);                                                /* It computes the summation over log (1+exp(wv_b)); */
    }

    return -b_v - sum;
}

/* It computes the free energy of a given label and a sample
Parameters: [m, y, *x]
m: RBM
y: label of unit
*x: input data array */
double FreeEnergy4DRBM(RBM *m, int y, gsl_vector *x)
{
    double F = 0.0, tmp = 0.0, aux;
    int j, i;

    F = gsl_vector_get(m->c, y);
    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        aux = 0.0;
        for (i = 0; i < m->n_visible_layer_neurons; i++) /* It computes W_{ij}*x_i */
            aux += gsl_vector_get(x, i) * gsl_matrix_get(m->W, i, j);
        aux += gsl_vector_get(m->b, j);    /* It computes computes W_{ij}*x_i + b_j */
        aux += gsl_matrix_get(m->U, y, j); /* It computes computes W_{ij}*x_i + b_j + U_{yj} */
        tmp += SoftPlus(aux);
    }
    F += tmp; /* It computes c+\sum_j softplus(Wx+U+b) */

    return F;
}

/* It computes the minimum square error among input and output
Parameters: [input, output]
input: input vector
output: output vector */
double getReconstructionError(gsl_vector *input, gsl_vector *output)
{
    int i;
    double error = 0.0;

    for (i = 0; i < input->size; i++)
        error += pow(gsl_vector_get(input, i) - gsl_vector_get(output, i), 2);
    return error / input->size;
}

/* It computes the probability of turning on a hidden unit j, as described by Equation 10
Parameters: [m, v]
m: RBM
v: visible units array */
gsl_vector *getProbabilityTurningOnHiddenUnit(RBM *m, gsl_vector *v)
{
    int i, j;
    gsl_vector *h = NULL;
    double tmp;

    h = gsl_vector_calloc(m->n_hidden_layer_neurons);
    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            tmp += (gsl_vector_get(v, i) * gsl_matrix_get(m->W, i, j));
        tmp += gsl_vector_get(m->b, j);
        tmp /= m->t;
        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(h, j, tmp);
    }

    return h;
}

/* It computes the probability of dropping out visible units and turning on a hidden unit j, as described by Equation 11
Parameters: [m, r, v]
m: RBM
r: hidden neurons dropout array
v: visible units array */
gsl_vector *getProbabilityDroppingVisibleUnitOut4TurningOnHiddenUnit(RBM *m, gsl_vector *r, gsl_vector *v)
{
    int i, j;
    gsl_vector *h = NULL;
    double tmp;

    h = gsl_vector_calloc(m->n_hidden_layer_neurons);

    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            tmp += (gsl_vector_get(v, i) * gsl_matrix_get(m->W, i, j));
        tmp += gsl_vector_get(m->b, j);
        tmp = SigmoidLogistic(tmp) * gsl_vector_get(r, j);
        gsl_vector_set(h, j, tmp);
    }

    return h;
}

/* It computes the probability of turning on a hidden unit j using a dropconnect mask
Parameters: [m, M, v]
m: RBM
M: dropconnect mask
v: visible units array */
gsl_vector *getProbabilityTurningOnHiddenUnit4Dropconnect(RBM *m, gsl_matrix *M, gsl_vector *v)
{
    int i, j;
    gsl_vector *h = NULL;
    double tmp;

    h = gsl_vector_calloc(m->n_hidden_layer_neurons);
    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            tmp += (gsl_vector_get(v, i) * gsl_matrix_get(m->W, i, j) * gsl_matrix_get(m->M, i, j));
        tmp += gsl_vector_get(m->b, j);
        tmp /= m->t;
        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(h, j, tmp);
    }

    return h;
}

/* It computes the probability of turning on a hidden unit j considering a DBM at bottom layer using Equation 22
Parameters: [m, v]
m: RBM
v: visible units vector */
gsl_vector *getProbabilityTurningOnHiddenUnit4DBM(RBM *m, gsl_vector *v)
{
    int i, j;
    gsl_vector *h = NULL;
    double tmp;

    h = gsl_vector_calloc(m->n_hidden_layer_neurons);
    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            tmp += (gsl_vector_get(v, i) * gsl_matrix_get(m->W, i, j) + gsl_vector_get(v, i) * gsl_matrix_get(m->W, i, j));
        tmp += gsl_vector_get(m->b, j);
        tmp /= m->t;
        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(h, j, tmp);
    }

    return h;
}

/* It computes the probability of turning on a hidden unit j using a dropconnect mask  considering a DBM at bottom layer using Equation 22
Parameters: [m, M, v]
m: RBM
M: dropconnect mask
v: visible units vector */
gsl_vector *getProbabilityTurningOnHiddenUnit4DBM4Dropconnect(RBM *m, gsl_matrix *M, gsl_vector *v)
{
    int i, j;
    gsl_vector *h = NULL;
    double tmp;

    h = gsl_vector_calloc(m->n_hidden_layer_neurons);
    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            tmp += (gsl_vector_get(v, i) * gsl_matrix_get(m->W, i, j) * gsl_matrix_get(m->M, i, j) + gsl_vector_get(v, i) * gsl_matrix_get(m->W, i, j) * gsl_matrix_get(m->M, i, j));
        tmp += gsl_vector_get(m->b, j);
        tmp /= m->t;
        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(h, j, tmp);
    }

    return h;
}

/* It computes the probability of dropping visible units for turning on a hidden unit j considering a DBM at bottom layer using Equation 22
Parameters: [m, r, v]
m: RBM
r: hidden neurons dropout array
v: visible units vector */
gsl_vector *getProbabilityDroppingVisibleUnitOut4TurningOnHiddenUnit4DBM(RBM *m, gsl_vector *r, gsl_vector *v)
{
    int i, j;
    gsl_vector *h = NULL;
    double tmp;

    h = gsl_vector_calloc(m->n_hidden_layer_neurons);
    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            tmp += (gsl_vector_get(v, i) * gsl_matrix_get(m->W, i, j) + gsl_vector_get(v, i) * gsl_matrix_get(m->W, i, j));
        tmp += gsl_vector_get(m->b, j);
        tmp /= m->t;
        tmp = SigmoidLogistic(tmp) * gsl_vector_get(r, j);
        gsl_vector_set(h, j, tmp);
    }

    return h;
}

/* It computes the probability of turning on a hidden unit - Fast version
Parameters: [m, v, prob_h]
m: RBM
v: visible units vector
prob_h: probability of hidden neurons */
void FASTgetProbabilityTurningOnHiddenUnit(RBM *m, gsl_vector *v, gsl_vector *prob_h)
{
    int i, j;
    double tmp;

    if (prob_h)
    {
        for (j = 0; j < m->n_hidden_layer_neurons; j++)
        {
            tmp = 0.0;
            for (i = 0; i < m->n_visible_layer_neurons; i++)
                tmp += (gsl_vector_get(v, i) * gsl_matrix_get(m->W, i, j));
            tmp += gsl_vector_get(m->b, j);
            tmp /= m->t;
            tmp = SigmoidLogistic(tmp);
            gsl_vector_set(prob_h, j, tmp);
        }
    }
    else
        fprintf(stderr, "\nThere is no prob_h vector allocated @FASTgetProbabilityTurningOnHiddenUnit.\n");
}

/* It computes the probability of turning on a hidden unit j for FPCD
Parameters: [m, v, fast_W]
m: RBM
v: visible units vector
fast_W: weight matrix for FPCD */
gsl_vector *getProbabilityTurningOnHiddenUnit4FPCD(RBM *m, gsl_vector *v, gsl_matrix *fast_W)
{
    int i, j;
    gsl_vector *h = NULL;
    double tmp;

    h = gsl_vector_calloc(m->n_hidden_layer_neurons);
    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            tmp += (gsl_vector_get(v, i) * (gsl_matrix_get(m->W, i, j) + gsl_matrix_get(fast_W, i, j)));
        tmp += gsl_vector_get(m->b, j);
        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(h, j, tmp);
    }

    return h;
}

/* It computes the probability of dropping out visible units and turning on a hidden unit j for FPCD
Parameters: [m, r, v, fast_W]
m: RBM
r: hidden units dropout array
v: visible units vector
fast_W: weight matrix for FPCD */
gsl_vector *getProbabilityDroppingVisibleUnitOut4TurningOnHiddenUnit4FPCD(RBM *m, gsl_vector *r, gsl_vector *v, gsl_matrix *fast_W)
{
    int i, j;
    gsl_vector *h = NULL;
    double tmp;

    h = gsl_vector_calloc(m->n_hidden_layer_neurons);
    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            tmp += (gsl_vector_get(v, i) * (gsl_matrix_get(m->W, i, j) + gsl_matrix_get(fast_W, i, j)));
        tmp += gsl_vector_get(m->b, j);
        tmp = SigmoidLogistic(tmp) * gsl_vector_get(m->r, j);
        gsl_vector_set(h, j, tmp);
    }

    return h;
}

/* It computes the probability of turning on a hidden unit j for FPCD with a dropconnect mask
Parameters: [m, M, v, fast_W]
m: RBM
M: dropconnect mask
v: visible units vector
fast_W: weight matrix for FPCD */
gsl_vector *getProbabilityTurningOnHiddenUnit4FPCD4Dropconnect(RBM *m, gsl_matrix *M, gsl_vector *v, gsl_matrix *fast_W)
{
    int i, j;
    gsl_vector *h = NULL;
    double tmp;

    h = gsl_vector_calloc(m->n_hidden_layer_neurons);
    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            tmp += ((gsl_vector_get(v, i) * (gsl_matrix_get(m->W, i, j) + gsl_matrix_get(fast_W, i, j))) * gsl_matrix_get(m->M, i, j));
        tmp += gsl_vector_get(m->b, j);
        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(h, j, tmp);
    }

    return h;
}

/* It computes the probability of turning on a visible unit j, as described by Equation 11
Parameters: [m, h]
m: RBM
h: hidden units array */
gsl_vector *getProbabilityTurningOnVisibleUnit(RBM *m, gsl_vector *h)
{
    int i, j;
    gsl_vector *v = NULL;
    double tmp;

    v = gsl_vector_calloc(m->n_visible_layer_neurons);

    for (j = 0; j < m->n_visible_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_hidden_layer_neurons; i++)
            tmp += (gsl_vector_get(h, i) * gsl_matrix_get(m->W, j, i));
        tmp += gsl_vector_get(m->a, j);
        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(v, j, tmp);
    }

    return v;
}

/* It computes the probability of dropping out hidden units and turning on a visible unit j, as described by Equation 11
Parameters: [m, r, h]
m: RBM
r: hidden units dropout array
h: hidden units vector */
gsl_vector *getProbabilityDroppingHiddenUnitOut4TurningOnVisibleUnit(RBM *m, gsl_vector *r, gsl_vector *h)
{
    int i, j;
    gsl_vector *v = NULL;
    double tmp;

    v = gsl_vector_calloc(m->n_visible_layer_neurons);

    for (j = 0; j < m->n_visible_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_hidden_layer_neurons; i++)
            tmp += (gsl_vector_get(h, i) * gsl_vector_get(r, i) * gsl_matrix_get(m->W, j, i));
        tmp += gsl_vector_get(m->a, j);
        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(v, j, tmp);
    }

    return v;
}

/* It computes the probability of turning on a visible unit j using a dropconnect mask
Parameters: [m, M, h]
m: RBM
M: dropconnect mask
h: hidden units array */
gsl_vector *getProbabilityTurningOnVisibleUnit4Dropconnect(RBM *m, gsl_matrix *M, gsl_vector *h)
{
    int i, j;
    gsl_vector *v = NULL;
    double tmp;

    v = gsl_vector_calloc(m->n_visible_layer_neurons);

    for (j = 0; j < m->n_visible_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_hidden_layer_neurons; i++)
            tmp += (gsl_vector_get(h, i) * gsl_matrix_get(m->W, j, i) * gsl_matrix_get(m->M, j, i));
        tmp += gsl_vector_get(m->a, j);
        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(v, j, tmp);
    }

    return v;
}

/* It computes the probability of turning on a visible unit j considering a DBM at top layer
Parameters: [m, h]
m: DBM
h: hidden units array */
gsl_vector *getProbabilityTurningOnVisibleUnit4DBM(RBM *m, gsl_vector *h)
{
    int i, j;
    gsl_vector *v = NULL;
    double tmp;

    v = gsl_vector_calloc(m->n_visible_layer_neurons);

    for (j = 0; j < m->n_visible_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_hidden_layer_neurons; i++)
            tmp += (gsl_vector_get(h, i) * gsl_matrix_get(m->W, j, i) + gsl_vector_get(h, i) * gsl_matrix_get(m->W, j, i));
        tmp += gsl_vector_get(m->a, j);
        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(v, j, tmp);
    }

    return v;
}

/* It computes the probability of turning on a visible unit j using a dropconnect mask considering a DBM at top layer
Parameters: [m, M, h]
m: DBM
M: dropconnect mask
h: hidden units array */
gsl_vector *getProbabilityTurningOnVisibleUnit4DBM4Dropconnect(RBM *m, gsl_matrix *M, gsl_vector *h)
{
    int i, j;
    gsl_vector *v = NULL;
    double tmp;

    v = gsl_vector_calloc(m->n_visible_layer_neurons);

    for (j = 0; j < m->n_visible_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_hidden_layer_neurons; i++)
            tmp += (gsl_vector_get(h, i) * gsl_matrix_get(m->W, j, i) * gsl_matrix_get(m->M, j, i) + gsl_vector_get(h, i) * gsl_matrix_get(m->W, j, i) * gsl_matrix_get(m->M, j, i));
        tmp += gsl_vector_get(m->a, j);
        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(v, j, tmp);
    }

    return v;
}

/* It computes the probability of dropping hidden units for turning on a visible unit j considering a DBM at top layer
Parameters: [m, r, h]
m: DBM
r: hidden units dropout array
h: hidden units array */
gsl_vector *getProbabilityDroppingHiddenUnitOut4TurningOnVisibleUnit4DBM(RBM *m, gsl_vector *r, gsl_vector *h)
{
    int i, j;
    gsl_vector *v = NULL;
    double tmp;

    v = gsl_vector_calloc(m->n_visible_layer_neurons);

    for (j = 0; j < m->n_visible_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_hidden_layer_neurons; i++)
            tmp += (gsl_vector_get(h, i) * gsl_vector_get(r, i) * gsl_matrix_get(m->W, j, i) + gsl_vector_get(h, i) * gsl_vector_get(r, i) * gsl_matrix_get(m->W, j, i));
        tmp += gsl_vector_get(m->a, j);
        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(v, j, tmp);
    }

    return v;
}

/* It computes the probability of turning on a visible unit j for FPCD
Parameters: [m, h, fast_W]
m: RBM
h: hidden units vector
fast_W: weight matrix for FPCD */
gsl_vector *getProbabilityTurningOnVisibleUnit4FPCD(RBM *m, gsl_vector *h, gsl_matrix *fast_W)
{
    int i, j;
    gsl_vector *v = NULL;
    double tmp;

    v = gsl_vector_calloc(m->n_visible_layer_neurons);

    for (j = 0; j < m->n_visible_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_hidden_layer_neurons; i++)
            tmp += (gsl_vector_get(h, i) * (gsl_matrix_get(m->W, j, i) + gsl_matrix_get(fast_W, j, i)));
        tmp += gsl_vector_get(m->a, j);
        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(v, j, tmp);
    }

    return v;
}

/* It computes the probability of dropping out hidden units and turning on a visible unit j for FPCD
Parameters: [m, r, h, fast_W]
m: RBM
r: hidden units dropout array
h: hidden units vector
fast_W: weight matrix for FPCD */
gsl_vector *getProbabilityDroppingHiddenUnitOut4TurningOnVisibleUnit4FPCD(RBM *m, gsl_vector *r, gsl_vector *h, gsl_matrix *fast_W)
{
    int i, j;
    gsl_vector *v = NULL;
    double tmp;

    v = gsl_vector_calloc(m->n_visible_layer_neurons);

    for (j = 0; j < m->n_visible_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_hidden_layer_neurons; i++)
            tmp += (gsl_vector_get(h, i) * gsl_vector_get(r, i) * (gsl_matrix_get(m->W, j, i) + gsl_matrix_get(fast_W, j, i)));
        tmp += gsl_vector_get(m->a, j);
        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(v, j, tmp);
    }

    return v;
}

/* It computes the probability of turning on a visible unit j for FPCD using a dropconnect mask
Parameters: [m, M, h, fast_W]
m: RBM
M: dropconnect mask
h: hidden units vector
fast_W: weight matrix for FPCD */
gsl_vector *getProbabilityTurningOnVisibleUnit4FPCD4Dropconnect(RBM *m, gsl_matrix *M, gsl_vector *h, gsl_matrix *fast_W)
{
    int i, j;
    gsl_vector *v = NULL;
    double tmp;

    v = gsl_vector_calloc(m->n_visible_layer_neurons);

    for (j = 0; j < m->n_visible_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_hidden_layer_neurons; i++)
            tmp += ((gsl_vector_get(h, i) * (gsl_matrix_get(m->W, j, i) + gsl_matrix_get(fast_W, j, i))) * gsl_matrix_get(m->M, j, i));
        tmp += gsl_vector_get(m->a, j);
        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(v, j, tmp);
    }

    return v;
}

/* It computes the probability of turning on a hidden unit j considering Gaussian RBMs
Parameters: [m, v, sigma]
m: DRBM
v: array of visible units
sigma: variance value */
gsl_vector *getProbabilityTurningOnHiddenUnit4Gaussian(RBM *m, gsl_vector *v, gsl_vector *sigma)
{
    int i, j;
    gsl_vector *h = NULL;
    double tmp;

    h = gsl_vector_calloc(m->n_hidden_layer_neurons);
    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            tmp += (-(gsl_vector_get(v, i) / gsl_vector_get(sigma, i))) * gsl_matrix_get(m->W, i, j);
        tmp = tmp - gsl_vector_get(m->b, j);
        tmp = 1.0 / (1.0 + exp(tmp));
        gsl_vector_set(h, j, tmp);
    }
    return h;
}

/* It computes the probability of turning on a hidden unit j considering Gaussian RBMs with Dropout
Parameters: [m, r, v, sigma]
m: DRBM
r: hidden neurons dropout vector
v: array of visible units
sigma: variance value */
gsl_vector *getProbabilityTurningOnHiddenUnit4Gaussian4Dropout(RBM *m, gsl_vector *r, gsl_vector *v, gsl_vector *sigma)
{
    int i, j;
    gsl_vector *h = NULL;
    double tmp;

    h = gsl_vector_calloc(m->n_hidden_layer_neurons);
    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_visible_layer_neurons; i++)
            tmp += (-(gsl_vector_get(v, i) / gsl_vector_get(sigma, i))) * gsl_matrix_get(m->W, i, j);
        tmp = tmp - gsl_vector_get(m->b, j);
        tmp = (1.0 / (1.0 + exp(tmp))) * gsl_vector_get(r, j);
        gsl_vector_set(h, j, tmp);
    }
    return h;
}

/* It computes the probability of turning on a visible unit i considering Gaussian RBMs
Parameters: [m, h, sigma]
m: DRBM
v: array of hidden units
sigma: variance value */
gsl_vector *getProbabilityTurningOnVisibleUnit4Gaussian(RBM *m, gsl_vector *h, gsl_vector *sigma)
{
    int i, j;
    gsl_vector *v = NULL;
    double tmp;

    v = gsl_vector_calloc(m->n_visible_layer_neurons);

    for (j = 0; j < m->n_visible_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_hidden_layer_neurons; i++)
            tmp += gsl_vector_get(h, i) * gsl_matrix_get(m->W, j, i);
        tmp = (tmp * gsl_vector_get(sigma, j)) + gsl_vector_get(m->a, j);
        gsl_vector_set(v, j, tmp);
    }
    return v;
}

/* It computes the probability of turning on a visible unit i considering Gaussian RBMs with Dropout
Parameters: [m, r, h, sigma]
m: DRBM
r: hidden neurons dropout vector
h: array of hidden units
sigma: variance value */
gsl_vector *getProbabilityTurningOnVisibleUnit4Gaussian4Dropout(RBM *m, gsl_vector *r, gsl_vector *h, gsl_vector *sigma)
{
    int i, j;
    gsl_vector *v = NULL;
    double tmp;

    v = gsl_vector_calloc(m->n_visible_layer_neurons);

    for (j = 0; j < m->n_visible_layer_neurons; j++)
    {
        tmp = 0.0;
        for (i = 0; i < m->n_hidden_layer_neurons; i++)
            tmp += gsl_vector_get(h, i) * gsl_vector_get(r, i) * gsl_matrix_get(m->W, j, i);
        tmp = ((tmp * gsl_vector_get(sigma, j)) + gsl_vector_get(m->a, j));
        gsl_vector_set(v, j, tmp);
    }
    return v;
}

/* It computes the probability of turning on a hidden unit j considering Discriminative RBMs with Bernoulli visible units, i..e, p(h|y,x)
Parameters: [m, *y]
m: RBM
*y: binary array */
gsl_vector *getDiscriminativeProbabilityTurningOnHiddenUnit(RBM *m, gsl_vector *y)
{
    int i, j;
    gsl_vector *h = NULL;
    double tmp, aux;

    h = gsl_vector_calloc(m->n_hidden_layer_neurons);
    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        tmp = aux = 0.0;
        for (i = 0; i < m->n_visible_layer_neurons; i++) /* It computes w_{ij}*v_i */
            tmp += (gsl_vector_get(m->v, i) * gsl_matrix_get(m->W, i, j));
        tmp += gsl_vector_get(m->b, j); /* It computes (w_{ij}*v_i)+b_j */

        /* It computes y*U_j */
        for (i = 0; i < m->n_labels; i++)
            aux += (gsl_matrix_get(m->U, i, j) * gsl_vector_get(y, i));

        /* It computes (w_{ij}*v_i)+b_j+(y*U_j) */
        tmp += aux;

        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(h, j, tmp);
    }

    return h;
}

/* It computes the probability of turning on a hidden unit j with Dropout considering Discriminative RBMs with Bernoulli visible units, i..e, p(h|y,x)
Parameters: [m, *r, *y]
m: RBM
*r: hidden neurons dropout vector
*y: binary array */
gsl_vector *getDiscriminativeProbabilityTurningOnHiddenUnit4Dropout(RBM *m, gsl_vector *r, gsl_vector *y)
{
    int i, j;
    gsl_vector *h = NULL;
    double tmp, aux;

    h = gsl_vector_calloc(m->n_hidden_layer_neurons);
    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        tmp = aux = 0.0;
        for (i = 0; i < m->n_visible_layer_neurons; i++) /* It computes w_{ij}*v_i */
            tmp += (gsl_vector_get(m->v, i) * gsl_matrix_get(m->W, i, j));
        tmp += gsl_vector_get(m->b, j); /* It computes (w_{ij}*v_i)+b_j */

        /* It computes y*U_j */
        for (i = 0; i < m->n_labels; i++)
            aux += (gsl_matrix_get(m->U, i, j) * gsl_vector_get(y, i));

        /* It computes (w_{ij}*v_i)+b_j+(y*U_j) */
        tmp += aux;

        tmp = SigmoidLogistic(tmp) * gsl_vector_get(r, j);
        gsl_vector_set(h, j, tmp);
    }

    return h;
}

/* It computes the probability of turning on a hidden unit j considering Discriminative RBMs and Gaussian visible units
Parameters: [m, y]
m: DRBM
y: array of label units */
gsl_vector *getDiscriminativeProbabilityTurningOnHiddenUnit4GaussianVisibleUnit(RBM *m, gsl_vector *y)
{
    int i, j;
    gsl_vector *h = NULL;
    double tmp, aux;

    h = gsl_vector_calloc(m->n_hidden_layer_neurons);
    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        tmp = aux = 0.0;
        for (i = 0; i < m->n_visible_layer_neurons; i++) /* It computes w_{ij}*v_i/sigma_i */
            tmp += ((gsl_vector_get(m->v, i) * gsl_matrix_get(m->W, i, j)) / gsl_vector_get(m->sigma, i));
        tmp += gsl_vector_get(m->b, j); /* It computes (w_{ij}*v_i)+b_j */

        /* It computes y*U_j */
        for (i = 0; i < m->n_labels; i++)
            aux += (gsl_matrix_get(m->U, i, j) * gsl_vector_get(y, i));

        /* It computes (w_{ij}*v_i)+b_j+(y*U_j) */
        tmp += aux;

        tmp = SigmoidLogistic(tmp);
        gsl_vector_set(h, j, tmp);
    }

    return h;
}

/* It computes the probability of turning on a hidden unit j with Dropout considering Discriminative RBMs and Gaussian visible units
Parameters: [m, r, y]
m: DRBM
r: hidden neurons dropout array
y: array of label units */
gsl_vector *getDiscriminativeProbabilityTurningOnHiddenUnit4GaussianVisibleUnit4Dropout(RBM *m, gsl_vector *r, gsl_vector *y)
{
    int i, j;
    gsl_vector *h = NULL;
    double tmp, aux;

    h = gsl_vector_calloc(m->n_hidden_layer_neurons);
    for (j = 0; j < m->n_hidden_layer_neurons; j++)
    {
        tmp = aux = 0.0;
        for (i = 0; i < m->n_visible_layer_neurons; i++) /* It computes w_{ij}*v_i/sigma_i */
            tmp += ((gsl_vector_get(m->v, i) * gsl_matrix_get(m->W, i, j)) / gsl_vector_get(m->sigma, i));
        tmp += gsl_vector_get(m->b, j); /* It computes (w_{ij}*v_i)+b_j */

        /* It computes y*U_j */
        for (i = 0; i < m->n_labels; i++)
            aux += (gsl_matrix_get(m->U, i, j) * gsl_vector_get(y, i));

        /* It computes (w_{ij}*v_i)+b_j+(y*U_j) */
        tmp += aux;

        tmp = SigmoidLogistic(tmp) * gsl_vector_get(r, j);
        gsl_vector_set(h, j, tmp);
    }

    return h;
}

/* It computes the probability of turning on a visible unit i considering Discriminative RBMs and Gaussian visible units
Parameters: [m, h]
m: DRBM
h: array of hidden units */
gsl_vector *getDiscriminativeProbabilityTurningOnVisibleUnit4GaussianVisibleUnit(RBM *m, gsl_vector *h)
{
    gsl_vector *v = NULL;
    int i, j;
    double tmp;
    const gsl_rng_type *T = NULL;
    gsl_rng *r = NULL;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    v = gsl_vector_calloc(m->n_visible_layer_neurons);

    for (i = 0; i < m->n_visible_layer_neurons; i++)
    {
        tmp = 0.0;
        for (j = 0; j < m->n_hidden_layer_neurons; j++)
            tmp += (gsl_vector_get(h, j) * gsl_matrix_get(m->W, i, j));
        tmp += gsl_vector_get(m->a, i);
        tmp = gsl_ran_gaussian(r, gsl_vector_get(m->sigma, i)) + tmp; /* Equation 13 of paper "Model Selection for Discriminative Restricted Boltzmann Machines Through Meta-heuristic Techniques" */
        gsl_vector_set(v, i, tmp);
    }
    gsl_rng_free(r);

    return v;
}

/* It computes the probability of turning on a visible unit i with Dropout considering Discriminative RBMs and Gaussian visible units
Parameters: [m, r, h]
m: DRBM
r: hidden neurons dropout vector
h: array of hidden units */
gsl_vector *getDiscriminativeProbabilityTurningOnVisibleUnit4GaussianVisibleUnit4Dropout(RBM *m, gsl_vector *r, gsl_vector *h)
{
    gsl_vector *v = NULL;
    int i, j;
    double tmp;
    const gsl_rng_type *T = NULL;
    gsl_rng *s = NULL;

    srand(time(NULL));
    T = gsl_rng_default;
    s = gsl_rng_alloc(T);
    gsl_rng_set(s, random_seed_deep());

    v = gsl_vector_calloc(m->n_visible_layer_neurons);

    for (i = 0; i < m->n_visible_layer_neurons; i++)
    {
        tmp = 0.0;
        for (j = 0; j < m->n_hidden_layer_neurons; j++)
            tmp += (gsl_vector_get(h, j) * gsl_vector_get(r, j) * gsl_matrix_get(m->W, i, j));
        tmp += gsl_vector_get(m->a, i);
        tmp = (gsl_ran_gaussian(s, gsl_vector_get(m->sigma, i)) + tmp);
        gsl_vector_set(v, i, tmp);
    }
    gsl_rng_free(s);

    return v;
}

/* It computes the probability of label unit (y) given the hidden (h) one, i.e., P(y|h)
Parameters: [m]
m: RBM */
gsl_vector *getDiscriminativeProbabilityLabelUnit(RBM *m)
{
    int j, k;
    double den = 0.0, tmp;
    gsl_vector *y = NULL;

    y = gsl_vector_calloc(m->n_labels);
    gsl_vector_set_zero(y);

    /* It computes \sum y* {\sum U_yj*h_j} + c_y */
    for (k = 0; k < m->n_labels; k++)
    {
        tmp = 0.0;
        for (j = 0; j < m->n_hidden_layer_neurons; j++) /* It computes \sum {U_yj*h_j} */
            tmp += (gsl_matrix_get(m->U, k, j) * gsl_vector_get(m->h, j));
        tmp += gsl_vector_get(m->c, k); /* It computes \sum {U_yj*h_j} + c_y */
        gsl_vector_set(y, k, exp(tmp));
        den += gsl_vector_get(y, k);
    }
    gsl_vector_scale(y, 1 / den);

    return y;
}

/* It computes the pseudo-likelihood of a sample x in an RBM, and it assumes x is a binary vector
Parameters: [m, x]
m: RBM
x: input sample
The above source-code was based on http://deeplearning.net/tutorial/rbm.html (equation log PL(x) at section "Proxies to Likelihood") */
double getPseudoLikelihood(RBM *m, gsl_vector *x)
{
    double pl;
    const gsl_rng_type *T = NULL;
    int index;
    gsl_rng *r = NULL;
    gsl_vector *x_flipped = NULL;

    srand(time(NULL));
    T = gsl_rng_default;
    r = gsl_rng_alloc(T);
    gsl_rng_set(r, random_seed_deep());

    x_flipped = gsl_vector_alloc(x->size);
    gsl_vector_memcpy(x_flipped, x);

    index = gsl_rng_uniform_int(r, (long int)m->n_visible_layer_neurons);   /* It generates the index of the bit to be flipped */
    gsl_vector_set(x_flipped, index, 1 - gsl_vector_get(x_flipped, index)); /* It flips the bit at index position */
    pl = m->n_visible_layer_neurons * log(SigmoidLogistic(FreeEnergy(m, x_flipped) - FreeEnergy(m, x)));

    gsl_rng_free(r);
    gsl_vector_free(x_flipped);

    return pl;
}
/**************************/
