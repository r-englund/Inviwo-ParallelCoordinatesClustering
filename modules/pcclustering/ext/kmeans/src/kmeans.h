// C++ implementation of K-Means clustering for n-dimensional data.
//
// This class follows the standard Expectation-Maximization iterative algorithm,
// also known by Lloyd's algorithm:
//   Repeat until convergence:
//      - Assign all points to the nearest cluster.
//      - Compute the cluster mean based on its assigned points.
//   Convergence is when the cluster assignment doesn't change.
//   The initialization of the means uses k random points from the data.
//
// Author: Felix Duvallet

#ifndef __KMEANS_KMEANS_H__
#define __KMEANS_KMEANS_H__

#include <map>
#include <string>
#include <vector>

#include "point.h"

class KMeans {
 public:
  // K is the number of clusters we want. Max iterations is just to prevent
  // running forever.
  KMeans(int k, int max_iterations = 100);

  // Copy all the given points.
  bool init(const std::vector<Point> &points);

  // Run until convergence.
  bool run();

  // Load the points from file and into the vector.
  static bool loadPoints(const std::string &filepath,
                         std::vector<Point> *points);

  void printMeans();

  // Write means to file, in the same format as the input file.
  void writeMeans(const std::string &filepath);

 protected:
  // Assign each point to the nearest cluster. Returns true if any point's
  // cluster assignment has changed, so we can detect convergence.
  bool assign();

  // Compute the means to be the average of all the points in each cluster.
  bool update_means();

  // Returns the index of the cluster nearest to this point.
  int findNearestCluster(const Point &point);

  // Computes a new cluster mean (output parameter mean) using the points in
  // that cluster. The multimap is mapping from cluster_id -> Point* (it is
  // generated by update_means).
  void computeClusterMean(
    const std::multimap<int, const Point *> &multimap,
    int cluster,
    Point *mean);

  // Number of clusters, the means, and all the points stored.
  int num_clusters_;
  int max_iterations_;
  std::vector<Point> means_;
  std::vector<Point> points_;

 public:
  const std::vector<Point> &getPoints() const {
    return points_;
  }

  const std::vector<Point> &getMeans() const {
    return means_;
  }
};

#endif  // __KMEANS_KMEANS_H__
