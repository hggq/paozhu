/*
 * Statistics Utilities - Pure statistical computation (no SVG dependency)
 *
 * KernelDensity: Kernel Density Estimation for visualizing data distributions
 *   - Does NOT assume normality; follows the actual shape of the data
 *   - Supports quantile computation via CDF bisection
 *   - Bandwidth auto-selection via Silverman's rule of thumb
 *
 * ClusteringProcess: Hierarchical K-Means clustering
 *   - Algorithm extracted from ECharts ecStat clustering module
 *   - Bisecting k-means: iteratively splits the cluster with highest SSE
 *   - Returns cluster assignments and centroids
 *
 * LinearRegression: Linear regression analysis
 *   - Algorithm extracted from ECharts ecStat regression module
 *   - Supports: linear, linearThroughOrigin, exponential, logarithmic, polynomial
 *   - Returns predicted values, parameters, and expression string
 */

#ifndef STATS_H
#define STATS_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <utility>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <limits>

// ============================================================
// Kernel Density Estimation
// ============================================================

class KernelDensity {
public:
    // Default constructor (empty data)
    KernelDensity() : bandwidth_(1.0), dataMin_(0), dataMax_(1) {}

    // Construct from raw data. bandwidth=0 triggers auto-selection (Silverman).
    KernelDensity(const std::vector<double>& data, double bandwidth = 0);

    // Estimated density at point x (Gaussian kernel)
    double density(double x) const;

    // CDF at point x (integrated from -infinity)
    double cdf(double x) const;

    // Quantile: inverse CDF via bisection
    double quantile(double p) const;

    // Left-tail interval: from quantile(0) to quantile(p)
    // Returns [min, quantile(p)]
    std::pair<double, double> leftTailInterval(double p) const;

    // Central interval: symmetric around median
    // e.g. p=0.2 → [quantile(0.4), quantile(0.6)]
    std::pair<double, double> centralInterval(double p) const;

    // Generate smooth curve points for plotting
    // x range: data min/max ± one bandwidth on each side
    std::vector<std::pair<double, double>> generateCurve(int numPoints = 100) const;

    // Accessors
    const std::vector<double>& data() const { return data_; }
    double bandwidth() const { return bandwidth_; }
    double dataMin() const { return dataMin_; }
    double dataMax() const { return dataMax_; }

    // Silverman's rule of thumb for bandwidth
    static double autoBandwidth(const std::vector<double>& data);

private:
    std::vector<double> data_;
    double bandwidth_;
    double dataMin_;
    double dataMax_;

    // Standard normal PDF
    static double normPdf(double x);
    // Standard normal CDF using std::erf
    static double normCdf(double x);
};

// ============================================================
// Hierarchical K-Means Clustering (Bisecting K-Means)
// Algorithm extracted from ECharts ecStat clustering module
// ============================================================

class ClusteringProcess {
public:
    // Cluster assignment result per data point
    struct ClusterAssignment {
        int clusterIndex;   // assigned cluster index (0-based)
        double distance;    // squared distance to centroid
    };

    // Result of clustering
    struct Result {
        std::vector<std::vector<double>> centroids;       // k centroids, each of dimension d
        std::vector<ClusterAssignment> clusterAssigned;    // assignment per data point
        std::vector<std::vector<std::vector<double>>> pointsInCluster; // points grouped by cluster
        bool isEnd = false;
    };

    // Dimension specification: {name, index}
    struct Dimension {
        std::string name;
        int index = 0;
    };

    // Config for clustering
    struct Config {
        int clusterCount = 2;                               // target number of clusters (>= 2)
        std::vector<int> dimensions;                        // which columns to use for distance
        int outputClusterIndexDimension = -1;               // column to store cluster index
        std::vector<int> outputCentroidDimensions;          // columns to store centroid values
        bool stepByStep = false;                            // if true, returns intermediate results
    };

    ClusteringProcess() = default;

    // Main entry point: run hierarchical k-means clustering
    // data: 2D array, each row is a data point with multiple columns
    // config: clustering parameters
    static Result run(const std::vector<std::vector<double>>& data, const Config& config);

    // Convenience: cluster raw 2D data (all columns used for distance)
    // data: 2D array where each row is [x, y, ...]
    // k: number of clusters
    // Returns: cluster assignments (index per data point) and centroids
    static Result simpleKMeans(const std::vector<std::vector<double>>& data, int k);

private:
    // Compute centroid (mean) of a set of points along specified dimensions
    static std::vector<double> computeCentroid(
        const std::vector<std::vector<double>>& points,
        const std::vector<int>& dims);

    // Squared Euclidean distance between a point and a centroid
    static double squaredDistance(
        const std::vector<double>& point,
        const std::vector<double>& centroid,
        const std::vector<int>& dims);

    // K-means on a subset of points (k=2), returns centroids and assignments
    struct KMeans2Result {
        std::vector<std::vector<double>> centroids;
        std::vector<ClusterAssignment> assignments;
    };
    static KMeans2Result kMeans2(
        const std::vector<std::vector<double>>& points,
        const std::vector<int>& dims);

    // Sum of squared distances for a set of points
    static double sse(const std::vector<std::vector<double>>& points,
                      const std::vector<int>& dims);

    // Random initialization of centroids within data extents
    static std::vector<std::vector<double>> randomCentroids(
        const std::vector<std::vector<double>>& points,
        int k, const std::vector<int>& dims);
};

// ============================================================
// Linear Regression
// Algorithm extracted from ECharts ecStat regression module
// Supports: linear, linearThroughOrigin, exponential, logarithmic, polynomial
// ============================================================

class LinearRegression {
public:
    // Regression method types
    enum Method {
        LINEAR,                  // y = a*x + b
        LINEAR_THROUGH_ORIGIN,   // y = a*x
        EXPONENTIAL,             // y = a * e^(b*x)
        LOGARITHMIC,             // y = a + b * ln(x)
        POLYNOMIAL,              // y = a0 + a1*x + a2*x^2 + ... + an*x^n
        MULTIVARIATE_LINEAR      // y = b0 + b1*x1 + b2*x2 + ... + bn*xn
    };

    // Regression parameters
    struct Parameters {
        double gradient = 0;     // slope (linear, log) or index (exponential)
        double intercept = 0;    // intercept (linear, log) or coefficient (exponential)
        std::vector<double> coefficients; // polynomial coefficients (a0, a1, ..., an)
    };

    // Result of regression
    struct Result {
        std::vector<std::vector<double>> points;  // predicted points [x, y]
        Parameters parameters;                     // regression parameters
        std::string expression;                    // human-readable formula
    };

    // Config for regression
    struct Config {
        Method method = LINEAR;
        std::vector<int> dimensions = {0, 1};      // [x_dim, y_dim] for simple regression
        std::vector<int> independentVars;           // indices of independent variables (multivariate)
        int dependentVar = -1;                      // index of dependent variable (-1 = last column)
        int order = 2;                              // polynomial order (for POLYNOMIAL only)
    };

    LinearRegression() = default;

    // Main entry point: run regression
    // data: 2D array where each row is [x, y, ...]
    // config: regression parameters
    static Result run(const std::vector<std::vector<double>>& data, const Config& config);

    // Convenience: simple linear regression on 2D data [[x1,y1], [x2,y2], ...]
    static Result linear(const std::vector<std::vector<double>>& data);

    // Convenience: polynomial regression of given order
    static Result polynomial(const std::vector<std::vector<double>>& data, int order);

    // Convenience: exponential regression
    static Result exponential(const std::vector<std::vector<double>>& data);

    // Convenience: logarithmic regression
    static Result logarithmic(const std::vector<std::vector<double>>& data);

    // Convenience: multivariate linear regression
    // data: each row is [x1, x2, ..., xn, y]
    // independentVars: which columns are independent variables
    // dependentVar: which column is the dependent variable (-1 = last column)
    static Result multivariate(const std::vector<std::vector<double>>& data,
                               const std::vector<int>& independentVars = {},
                               int dependentVar = -1);

private:
    // Ordinary Least Squares: y = a*x + b
    static Result olsLinear(const std::vector<std::vector<double>>& data,
                            int xDim, int yDim);

    // Linear through origin: y = a*x
    static Result olsLinearThroughOrigin(const std::vector<std::vector<double>>& data,
                                         int xDim, int yDim);

    // Exponential: y = a * e^(b*x) via log transform
    static Result olsExponential(const std::vector<std::vector<double>>& data,
                                 int xDim, int yDim);

    // Logarithmic: y = a + b * ln(x)
    static Result olsLogarithmic(const std::vector<std::vector<double>>& data,
                                 int xDim, int yDim);

    // Polynomial: y = a0 + a1*x + ... + an*x^n via Gaussian elimination
    static Result olsPolynomial(const std::vector<std::vector<double>>& data,
                                int xDim, int yDim, int order);

    // Multivariate linear: y = b0 + b1*x1 + ... + bn*xn via Normal Equation
    static Result olsMultivariate(const std::vector<std::vector<double>>& data,
                                  const std::vector<int>& independentVars,
                                  int dependentVar);

    // Gaussian elimination for solving linear equations
    static std::vector<double> gaussianEliminate(std::vector<std::vector<double>> matrix,
                                                  int n);

    // Round to given decimal places
    static double roundTo(double val, int decimals);
};

#endif // STATS_H