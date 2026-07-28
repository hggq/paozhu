/*
 * Statistics Utilities Implementation
 */

#include "svg_stats.h"

// ============================================================
// KernelDensity Implementation
// ============================================================

KernelDensity::KernelDensity(const std::vector<double>& data, double bandwidth)
    : data_(data) {
    if (data_.empty()) {
        bandwidth_ = 1.0;
        dataMin_ = 0;
        dataMax_ = 1;
        return;
    }

    auto [minIt, maxIt] = std::minmax_element(data_.begin(), data_.end());
    dataMin_ = *minIt;
    dataMax_ = *maxIt;

    bandwidth_ = (bandwidth > 0) ? bandwidth : autoBandwidth(data_);
}

double KernelDensity::normPdf(double x) {
    return 0.3989422804014327 * std::exp(-0.5 * x * x); // 1/sqrt(2π)
}

double KernelDensity::normCdf(double x) {
    return 0.5 * (1.0 + std::erf(x * 0.7071067811865476)); // x / sqrt(2)
}

double KernelDensity::density(double x) const {
    if (data_.empty()) return 0.0;
    double sum = 0.0;
    double invH = 1.0 / bandwidth_;
    for (double xi : data_) {
        sum += normPdf((x - xi) * invH);
    }
    return sum * invH / static_cast<double>(data_.size());
}

double KernelDensity::cdf(double x) const {
    if (data_.empty()) return 0.0;
    double sum = 0.0;
    double invH = 1.0 / bandwidth_;
    for (double xi : data_) {
        sum += normCdf((x - xi) * invH);
    }
    return sum / static_cast<double>(data_.size());
}

double KernelDensity::quantile(double p) const {
    if (data_.empty()) return 0.0;
    p = std::max(0.0, std::min(1.0, p));

    // Bisection search
    double lo = dataMin_ - 3.0 * bandwidth_;
    double hi = dataMax_ + 3.0 * bandwidth_;

    for (int iter = 0; iter < 50; ++iter) {
        double mid = (lo + hi) * 0.5;
        double c = cdf(mid);
        if (c < p) {
            lo = mid;
        } else {
            hi = mid;
        }
    }
    return (lo + hi) * 0.5;
}

std::pair<double, double> KernelDensity::leftTailInterval(double p) const {
    return {dataMin_, quantile(p)};
}

std::pair<double, double> KernelDensity::centralInterval(double p) const {
    double tail = (1.0 - p) * 0.5;
    return {quantile(tail), quantile(1.0 - tail)};
}

std::vector<std::pair<double, double>> KernelDensity::generateCurve(int numPoints) const {
    std::vector<std::pair<double, double>> result;
    if (data_.empty() || numPoints <= 0) return result;

    double xMin = dataMin_ - bandwidth_;
    double xMax = dataMax_ + bandwidth_;
    double step = (xMax - xMin) / (numPoints - 1);

    for (int i = 0; i < numPoints; ++i) {
        double x = xMin + step * i;
        result.emplace_back(x, density(x));
    }
    return result;
}

double KernelDensity::autoBandwidth(const std::vector<double>& data) {
    if (data.size() < 2) return 1.0;

    double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    double variance = 0.0;
    for (double x : data) {
        double diff = x - mean;
        variance += diff * diff;
    }
    double stddev = std::sqrt(variance / data.size());

    // Silverman's rule of thumb
    double n = static_cast<double>(data.size());
    double silverman = 1.06 * stddev * std::pow(n, -0.2);

    // Fallback: use IQR if stddev is near zero
    if (silverman < 1e-9) {
        std::vector<double> sorted = data;
        std::sort(sorted.begin(), sorted.end());
        double q1 = sorted[sorted.size() / 4];
        double q3 = sorted[3 * sorted.size() / 4];
        double iqr = q3 - q1;
        silverman = 0.9 * iqr * std::pow(n, -0.2);
    }

    return std::max(silverman, 1e-9);
}

// ============================================================
// ClusteringProcess Implementation
// ============================================================

std::vector<double> ClusteringProcess::computeCentroid(
    const std::vector<std::vector<double>>& points,
    const std::vector<int>& dims) {
    if (points.empty()) return {};
    std::vector<double> centroid(dims.size(), 0.0);
    for (size_t i = 0; i < dims.size(); ++i) {
        double sum = 0.0;
        for (const auto& p : points) {
            sum += p[dims[i]];
        }
        centroid[i] = sum / points.size();
    }
    return centroid;
}

double ClusteringProcess::squaredDistance(
    const std::vector<double>& point,
    const std::vector<double>& centroid,
    const std::vector<int>& dims) {
    double dist = 0.0;
    for (size_t i = 0; i < dims.size(); ++i) {
        double diff = point[dims[i]] - centroid[i];
        dist += diff * diff;
    }
    return dist;
}

double ClusteringProcess::sse(
    const std::vector<std::vector<double>>& points,
    const std::vector<int>& dims) {
    if (points.empty()) return 0.0;
    auto centroid = computeCentroid(points, dims);
    double sum = 0.0;
    for (const auto& p : points) {
        sum += squaredDistance(p, centroid, dims);
    }
    return sum;
}

std::vector<std::vector<double>> ClusteringProcess::randomCentroids(
    const std::vector<std::vector<double>>& points,
    int k, const std::vector<int>& dims) {
    if (points.empty()) return {};

    // Compute min/max for each dimension
    std::vector<double> mins(dims.size(), std::numeric_limits<double>::max());
    std::vector<double> maxs(dims.size(), std::numeric_limits<double>::lowest());
    for (const auto& p : points) {
        for (size_t i = 0; i < dims.size(); ++i) {
            double val = p[dims[i]];
            if (val < mins[i]) mins[i] = val;
            if (val > maxs[i]) maxs[i] = val;
        }
    }

    std::vector<std::vector<double>> centroids(k, std::vector<double>(dims.size()));
    for (int c = 0; c < k; ++c) {
        for (size_t i = 0; i < dims.size(); ++i) {
            double span = maxs[i] - mins[i];
            double r = static_cast<double>(rand()) / RAND_MAX;
            centroids[c][i] = mins[i] + span * r;
        }
    }
    return centroids;
}

ClusteringProcess::KMeans2Result ClusteringProcess::kMeans2(
    const std::vector<std::vector<double>>& points,
    const std::vector<int>& dims) {
    KMeans2Result result;
    size_t n = points.size();

    // Initialize centroids by picking two random data points
    // This is more robust than random points within extents,
    // especially for small/tight clusters
    result.centroids.resize(2, std::vector<double>(dims.size(), 0.0));
    if (n >= 2) {
        int idx0 = rand() % n;
        int idx1 = rand() % n;
        while (idx1 == idx0 && n > 1) {
            idx1 = rand() % n;
        }
        for (size_t d = 0; d < dims.size(); ++d) {
            result.centroids[0][d] = points[idx0][dims[d]];
            result.centroids[1][d] = points[idx1][dims[d]];
        }
    } else if (n == 1) {
        for (size_t d = 0; d < dims.size(); ++d) {
            result.centroids[0][d] = points[0][dims[d]];
            result.centroids[1][d] = points[0][dims[d]] + 1.0;
        }
    }

    result.assignments.resize(n);

    bool changed = true;
    int maxIter = 100;
    while (changed && maxIter-- > 0) {
        changed = false;

        // Assign points to nearest centroid
        for (size_t i = 0; i < n; ++i) {
            double bestDist = std::numeric_limits<double>::max();
            int bestIdx = 0;
            for (int c = 0; c < 2; ++c) {
                double d = squaredDistance(points[i], result.centroids[c], dims);
                if (d < bestDist) {
                    bestDist = d;
                    bestIdx = c;
                }
            }
            if (result.assignments[i].clusterIndex != bestIdx) {
                changed = true;
            }
            result.assignments[i].clusterIndex = bestIdx;
            result.assignments[i].distance = bestDist;
        }

        // Recompute centroids
        for (int c = 0; c < 2; ++c) {
            std::vector<std::vector<double>> clusterPoints;
            for (size_t i = 0; i < n; ++i) {
                if (result.assignments[i].clusterIndex == c) {
                    clusterPoints.push_back(points[i]);
                }
            }
            if (!clusterPoints.empty()) {
                result.centroids[c] = computeCentroid(clusterPoints, dims);
            }
        }
    }

    return result;
}

ClusteringProcess::Result ClusteringProcess::run(
    const std::vector<std::vector<double>>& data,
    const Config& config) {
    Result result;
    if (data.empty() || config.clusterCount < 2) {
        result.isEnd = true;
        return result;
    }

    int k = config.clusterCount;
    const auto& dims = config.dimensions;
    size_t n = data.size();

    if (dims.empty()) {
        result.isEnd = true;
        return result;
    }

    // Start with one cluster: centroid = mean of all data
    auto initialCentroid = computeCentroid(data, dims);
    std::vector<std::vector<double>> centroids = {initialCentroid};

    // Assignments for each data point
    result.clusterAssigned.resize(n);
    for (size_t i = 0; i < n; ++i) {
        result.clusterAssigned[i].clusterIndex = 0;
        result.clusterAssigned[i].distance = squaredDistance(data[i], initialCentroid, dims);
    }

    // Hierarchical splitting: bisecting k-means
    while (static_cast<int>(centroids.size()) < k) {
        double bestSSE = std::numeric_limits<double>::max();
        int bestSplitIdx = -1;
        KMeans2Result bestSplit;
        std::vector<std::vector<double>> bestSplitPoints;

        // Try splitting each existing cluster
        for (int c = 0; c < static_cast<int>(centroids.size()); ++c) {
            // Collect points in cluster c
            std::vector<std::vector<double>> clusterPoints;
            for (size_t i = 0; i < n; ++i) {
                if (result.clusterAssigned[i].clusterIndex == c) {
                    clusterPoints.push_back(data[i]);
                }
            }
            if (clusterPoints.size() < 2) continue;

            // Try k-means with 2 clusters
            auto split = kMeans2(clusterPoints, dims);

            // Compute SSE of the split
            double sse0 = 0.0, sse1 = 0.0;
            std::vector<std::vector<double>> pts0, pts1;
            for (size_t i = 0; i < clusterPoints.size(); ++i) {
                if (split.assignments[i].clusterIndex == 0) {
                    pts0.push_back(clusterPoints[i]);
                    sse0 += split.assignments[i].distance;
                } else {
                    pts1.push_back(clusterPoints[i]);
                    sse1 += split.assignments[i].distance;
                }
            }

            if (pts0.empty() || pts1.empty()) continue;

            double totalSSE = sse0 + sse1;
            if (totalSSE < bestSSE) {
                bestSSE = totalSSE;
                bestSplitIdx = c;
                bestSplit = split;
                bestSplitPoints = clusterPoints;
            }
        }

        if (bestSplitIdx < 0) break;

        // Apply the best split: replace cluster bestSplitIdx with two new clusters
        centroids[bestSplitIdx] = bestSplit.centroids[0];
        int newClusterIdx = static_cast<int>(centroids.size());
        centroids.push_back(bestSplit.centroids[1]);

        // Update assignments for points that were in the split cluster
        size_t splitPtIdx = 0;
        for (size_t i = 0; i < n; ++i) {
            if (result.clusterAssigned[i].clusterIndex == bestSplitIdx) {
                int assign = bestSplit.assignments[splitPtIdx].clusterIndex;
                if (assign == 0) {
                    // stays at bestSplitIdx
                } else {
                    result.clusterAssigned[i].clusterIndex = newClusterIdx;
                }
                result.clusterAssigned[i].distance = bestSplit.assignments[splitPtIdx].distance;
                ++splitPtIdx;
            }
        }
    }

    result.centroids = centroids;

    // Group points by cluster
    result.pointsInCluster.resize(centroids.size());
    for (size_t i = 0; i < n; ++i) {
        int ci = result.clusterAssigned[i].clusterIndex;
        if (ci >= 0 && ci < static_cast<int>(result.pointsInCluster.size())) {
            result.pointsInCluster[ci].push_back(data[i]);
        }
    }

    result.isEnd = true;
    return result;
}

ClusteringProcess::Result ClusteringProcess::simpleKMeans(
    const std::vector<std::vector<double>>& data, int k) {
    Config config;
    config.clusterCount = k;
    if (!data.empty()) {
        for (size_t i = 0; i < data[0].size(); ++i) {
            config.dimensions.push_back(static_cast<int>(i));
        }
    }
    return run(data, config);
}

// ============================================================
// LinearRegression Implementation
// ============================================================

std::vector<double> LinearRegression::gaussianEliminate(
    std::vector<std::vector<double>> matrix, int n) {
    // matrix is (n+1) x n, last column is the RHS
    // Forward elimination
    for (int i = 0; i < n - 1; ++i) {
        // Pivot: find max in column
        int maxRow = i;
        double maxVal = std::abs(matrix[i][i]);
        for (int r = i + 1; r < n; ++r) {
            if (std::abs(matrix[r][i]) > maxVal) {
                maxVal = std::abs(matrix[r][i]);
                maxRow = r;
            }
        }
        // Swap rows
        if (maxRow != i) {
            std::swap(matrix[i], matrix[maxRow]);
        }

        for (int r = i + 1; r < n; ++r) {
            double factor = matrix[r][i] / matrix[i][i];
            for (int c = i; c <= n; ++c) {
                matrix[r][c] -= factor * matrix[i][c];
            }
        }
    }

    // Back substitution
    std::vector<double> result(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = matrix[i][n];
        for (int j = i + 1; j < n; ++j) {
            sum -= matrix[i][j] * result[j];
        }
        result[i] = sum / matrix[i][i];
    }
    return result;
}

double LinearRegression::roundTo(double val, int decimals) {
    double factor = std::pow(10.0, decimals);
    return std::round(val * factor) / factor;
}

LinearRegression::Result LinearRegression::olsLinear(
    const std::vector<std::vector<double>>& data,
    int xDim, int yDim) {
    Result result;
    size_t n = data.size();
    if (n < 2) return result;

    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    for (const auto& row : data) {
        double x = row[xDim];
        double y = row[yDim];
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }

    double dn = static_cast<double>(n);
    double gradient = (dn * sumXY - sumX * sumY) / (dn * sumX2 - sumX * sumX);
    double intercept = sumY / dn - gradient * sumX / dn;

    result.parameters.gradient = gradient;
    result.parameters.intercept = intercept;

    // Generate predicted points
    result.points.reserve(n);
    for (const auto& row : data) {
        double x = row[xDim];
        double yPred = gradient * x + intercept;
        result.points.push_back({x, yPred});
    }

    // Build expression
    std::ostringstream oss;
    oss << "y = " << roundTo(gradient, 2) << "x";
    if (intercept >= 0) oss << " + " << roundTo(intercept, 2);
    else oss << " - " << roundTo(-intercept, 2);
    result.expression = oss.str();

    return result;
}

LinearRegression::Result LinearRegression::olsLinearThroughOrigin(
    const std::vector<std::vector<double>>& data,
    int xDim, int yDim) {
    Result result;
    size_t n = data.size();
    if (n < 2) return result;

    double sumX2 = 0, sumXY = 0;
    for (const auto& row : data) {
        double x = row[xDim];
        double y = row[yDim];
        sumX2 += x * x;
        sumXY += x * y;
    }

    double gradient = (sumX2 != 0) ? sumXY / sumX2 : 0;

    result.parameters.gradient = gradient;
    result.parameters.intercept = 0;

    result.points.reserve(n);
    for (const auto& row : data) {
        double x = row[xDim];
        result.points.push_back({x, gradient * x});
    }

    std::ostringstream oss;
    oss << "y = " << roundTo(gradient, 2) << "x";
    result.expression = oss.str();

    return result;
}

LinearRegression::Result LinearRegression::olsExponential(
    const std::vector<std::vector<double>>& data,
    int xDim, int yDim) {
    Result result;
    size_t n = data.size();
    if (n < 2) return result;

    // Algorithm from ECharts ecStat: y = a * e^(b*x)
    // Uses weighted least squares minimization on raw (x, y) data
    double sumY = 0, sumXY = 0, sumX2Y = 0, sumYLogY = 0, sumXYLogY = 0;
    for (const auto& row : data) {
        double x = row[xDim];
        double y = row[yDim];
        if (y <= 0) continue; // skip non-positive y for log transform
        sumY += y;
        sumXY += x * y;
        sumX2Y += x * x * y;
        double yLogY = y * std::log(y);
        sumYLogY += yLogY;
        sumXYLogY += x * yLogY;
    }

    double denom = sumY * sumX2Y - sumXY * sumXY;
    if (std::abs(denom) < 1e-12) return result;

    double logA = (sumX2Y * sumYLogY - sumXY * sumXYLogY) / denom;
    double b = (sumY * sumXYLogY - sumXY * sumYLogY) / denom;

    double a = std::exp(logA);

    result.parameters.gradient = b;
    result.parameters.intercept = a;

    result.points.reserve(n);
    for (const auto& row : data) {
        double x = row[xDim];
        result.points.push_back({x, a * std::exp(b * x)});
    }

    std::ostringstream oss;
    oss << "y = " << roundTo(a, 2) << "e^(" << roundTo(b, 2) << "x)";
    result.expression = oss.str();

    return result;
}

LinearRegression::Result LinearRegression::olsLogarithmic(
    const std::vector<std::vector<double>>& data,
    int xDim, int yDim) {
    Result result;
    size_t n = data.size();
    if (n < 2) return result;

    // Transform: y = a + b * ln(x)
    double sumLogX = 0, sumY = 0, sumLogX2 = 0, sumYLogX = 0;
    for (const auto& row : data) {
        double x = row[xDim];
        if (x <= 0) continue; // skip non-positive x for log
        double logX = std::log(x);
        double y = row[yDim];
        sumLogX += logX;
        sumY += y;
        sumLogX2 += logX * logX;
        sumYLogX += y * logX;
    }

    double dn = static_cast<double>(n);
    double denom = dn * sumLogX2 - sumLogX * sumLogX;
    if (std::abs(denom) < 1e-12) return result;

    double b = (dn * sumYLogX - sumY * sumLogX) / denom;
    double a = (sumY - b * sumLogX) / dn;

    result.parameters.gradient = b;
    result.parameters.intercept = a;

    result.points.reserve(n);
    for (const auto& row : data) {
        double x = row[xDim];
        double yPred = (x > 0) ? a + b * std::log(x) : a;
        result.points.push_back({x, yPred});
    }

    std::ostringstream oss;
    oss << "y = " << roundTo(a, 2);
    if (b >= 0) oss << " + " << roundTo(b, 2) << "ln(x)";
    else oss << " - " << roundTo(-b, 2) << "ln(x)";
    result.expression = oss.str();

    return result;
}

LinearRegression::Result LinearRegression::olsPolynomial(
    const std::vector<std::vector<double>>& data,
    int xDim, int yDim, int order) {
    Result result;
    size_t n = data.size();
    if (n < 2) return result;

    int s = order + 1; // number of coefficients
    // Build matrix: A[i][j] = sum(x^(i+j)), B[i] = sum(y * x^i)
    std::vector<std::vector<double>> matrix(s, std::vector<double>(s + 1, 0.0));

    // Precompute x^0..x^(2*order) for each point
    for (const auto& row : data) {
        double x = row[xDim];
        double y = row[yDim];
        double xp = 1.0;
        for (int i = 0; i < s; ++i) {
            // B[i] = sum(y * x^i)
            matrix[i][s] += y * xp;
            double xq = xp;
            for (int j = 0; j < s; ++j) {
                matrix[i][j] += xq;
                xq *= x;
            }
            xp *= x;
        }
    }

    auto coeffs = gaussianEliminate(matrix, s);

    result.parameters.coefficients = coeffs;

    result.points.reserve(n);
    for (const auto& row : data) {
        double x = row[xDim];
        double yPred = 0.0;
        double xp = 1.0;
        for (int i = 0; i < s; ++i) {
            yPred += coeffs[i] * xp;
            xp *= x;
        }
        result.points.push_back({x, yPred});
    }

    // Build expression
    std::ostringstream oss;
    oss << "y = ";
    for (int i = s - 1; i >= 0; --i) {
        double c = coeffs[i];
        if (i == static_cast<int>(s - 1)) {
            oss << roundTo(c, i + 1);
        } else {
            if (c >= 0) oss << " + ";
            else { oss << " - "; c = -c; }
            oss << roundTo(c, i + 1);
        }
        if (i > 1) oss << "x^" << i;
        else if (i == 1) oss << "x";
    }
    result.expression = oss.str();

    return result;
}

LinearRegression::Result LinearRegression::run(
    const std::vector<std::vector<double>>& data,
    const Config& config) {
    int xDim = config.dimensions.size() > 0 ? config.dimensions[0] : 0;
    int yDim = config.dimensions.size() > 1 ? config.dimensions[1] : 1;

    switch (config.method) {
        case LINEAR:
            return olsLinear(data, xDim, yDim);
        case LINEAR_THROUGH_ORIGIN:
            return olsLinearThroughOrigin(data, xDim, yDim);
        case EXPONENTIAL:
            return olsExponential(data, xDim, yDim);
        case LOGARITHMIC:
            return olsLogarithmic(data, xDim, yDim);
        case POLYNOMIAL:
            return olsPolynomial(data, xDim, yDim, config.order);
        case MULTIVARIATE_LINEAR: {
            // Determine independent vars and dependent var
            std::vector<int> indep = config.independentVars;
            int dep = config.dependentVar;
            if (indep.empty() && !data.empty()) {
                // Default: all columns except last are independent, last is dependent
                dep = (dep < 0) ? static_cast<int>(data[0].size()) - 1 : dep;
                for (int i = 0; i < dep; ++i) indep.push_back(i);
            }
            return olsMultivariate(data, indep, dep);
        }
        default:
            return olsLinear(data, xDim, yDim);
    }
}

LinearRegression::Result LinearRegression::linear(
    const std::vector<std::vector<double>>& data) {
    Config config;
    config.method = LINEAR;
    return run(data, config);
}

LinearRegression::Result LinearRegression::polynomial(
    const std::vector<std::vector<double>>& data, int order) {
    Config config;
    config.method = POLYNOMIAL;
    config.order = order;
    return run(data, config);
}

LinearRegression::Result LinearRegression::exponential(
    const std::vector<std::vector<double>>& data) {
    Config config;
    config.method = EXPONENTIAL;
    return run(data, config);
}

LinearRegression::Result LinearRegression::logarithmic(
    const std::vector<std::vector<double>>& data) {
    Config config;
    config.method = LOGARITHMIC;
    return run(data, config);
}

LinearRegression::Result LinearRegression::multivariate(
    const std::vector<std::vector<double>>& data,
    const std::vector<int>& independentVars,
    int dependentVar) {
    Config config;
    config.method = MULTIVARIATE_LINEAR;
    config.independentVars = independentVars;
    config.dependentVar = dependentVar;
    return run(data, config);
}

// ============================================================
// Multivariate Linear Regression Implementation
// Normal Equation: β = (XᵀX)⁻¹ Xᵀy
// ============================================================

LinearRegression::Result LinearRegression::olsMultivariate(
    const std::vector<std::vector<double>>& data,
    const std::vector<int>& independentVars,
    int dependentVar) {
    Result result;
    size_t n = data.size();
    if (n < 2 || independentVars.empty()) return result;

    int p = static_cast<int>(independentVars.size()); // number of independent variables
    int m = p + 1;  // including intercept term

    // Build design matrix X (n x m): first column is all 1s (intercept)
    // X[i][0] = 1, X[i][j+1] = data[i][independentVars[j]]
    // Build target vector y (n x 1)
    std::vector<std::vector<double>> XtX(m, std::vector<double>(m, 0.0));
    std::vector<double> Xty(m, 0.0);

    for (size_t i = 0; i < n; ++i) {
        const auto& row = data[i];
        double yi = row[dependentVar];

        // Build row vector: [1, x1, x2, ..., xp]
        std::vector<double> xi(m, 1.0);
        for (int j = 0; j < p; ++j) {
            xi[j + 1] = row[independentVars[j]];
        }

        // Accumulate XᵀX and Xᵀy
        for (int r = 0; r < m; ++r) {
            for (int c = 0; c < m; ++c) {
                XtX[r][c] += xi[r] * xi[c];
            }
            Xty[r] += xi[r] * yi;
        }
    }

    // Solve (XᵀX)β = Xᵀy via Gaussian elimination
    // Build augmented matrix [XᵀX | Xᵀy]
    std::vector<std::vector<double>> matrix(m, std::vector<double>(m + 1, 0.0));
    for (int r = 0; r < m; ++r) {
        for (int c = 0; c < m; ++c) {
            matrix[r][c] = XtX[r][c];
        }
        matrix[r][m] = Xty[r];
    }

    auto beta = gaussianEliminate(matrix, m); // beta = [b0, b1, b2, ..., bp]

    result.parameters.coefficients = beta;

    // Generate predicted values
    result.points.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        const auto& row = data[i];
        double yPred = beta[0]; // intercept
        for (int j = 0; j < p; ++j) {
            yPred += beta[j + 1] * row[independentVars[j]];
        }
        double yActual = row[dependentVar];
        // For multivariate: points store [yActual, yPred] for predicted-vs-actual plot
        result.points.push_back({yActual, yPred});
    }

    // Build expression
    std::ostringstream oss;
    oss << "y = " << roundTo(beta[0], 2);
    for (int j = 0; j < p; ++j) {
        double bj = beta[j + 1];
        if (bj >= 0) oss << " + ";
        else { oss << " - "; bj = -bj; }
        oss << roundTo(bj, 2) << "·x" << (j + 1);
    }
    result.expression = oss.str();

    return result;
}