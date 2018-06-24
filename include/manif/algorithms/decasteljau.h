#ifndef _MANIF_MANIF_DECASTELJAU_H_
#define _MANIF_MANIF_DECASTELJAU_H_

#include "manif/impl/manifold_base.h"

namespace manif
{

/**
 * @brief Curve fitting using the DeCasteljau algorithm
 * on Lie groups.
 *
 * @param trajectory, a discretized trajectory.
 * @param degree, the degree of smoothness of the fitted curve
 * @param k_interp, the number of points to interpolate
 * between two consecutive points of the trajectory
 * @return The interpolated smooth trajectory
 *
 * @note A naive implementation of the DeCasteljau algorithm
 * on Lie groups.
 *
 * https://www.wikiwand.com/en/De_Casteljau%27s_algorithm
 */
template <typename Manifold>
std::vector<typename Manifold::Manifold>
decasteljau(const std::vector<Manifold>& trajectory,
            const unsigned int degree,
            const unsigned int k_interp,
            const bool closed_curve = false)
{
  MANIF_CHECK(trajectory.size() > 2, "Oups");
  MANIF_CHECK(degree <= trajectory.size(), "Oups");
  MANIF_CHECK(k_interp > 0, "Oups");

  // Number of connected, non-overlapping segments
  const unsigned int n_segments =
      std::floor(double(trajectory.size()-degree)/(degree-1)+1);

  std::vector<std::vector<const Manifold*>> segments_control_points;
  for (unsigned int t=0; t<n_segments; ++t)
  {
    segments_control_points.emplace_back(std::vector<const Manifold*>());

    // Retrieve control points of the current segment
    for (int n=0; n<degree; ++n)
    {
      segments_control_points.back().push_back( &trajectory[t*(degree-1)+n] );
    }
  }

  const int last_pts_idx = (n_segments)*(degree-1);

  // Close the curve if there are left-over points
  if (closed_curve && last_pts_idx <= trajectory.size()-1)
  {
    const int left_over = trajectory.size()-1-last_pts_idx;
    segments_control_points.emplace_back(std::vector<const Manifold*>());

    // Get the left-over points
    for (int p=last_pts_idx; p<trajectory.size(); ++p)
    {
      segments_control_points.back().push_back( &trajectory[p] );
    }
    // Add a extra points from the beginning of the trajectory
    for (int p=0; p<degree-left_over-1; ++p)
    {
      segments_control_points.back().push_back( &trajectory[p] );
    }
  }

  const int segment_k_interp = (degree == 2) ?
        k_interp : k_interp * degree;

  // Actual curve fitting
  std::vector<Manifold> curve;
  for (unsigned int s=0; s<segments_control_points.size(); ++s)
  {
    for (int t=1; t<=segment_k_interp; ++t)
    {
      // t in [0,1]
      const double t_01 = static_cast<double>(t)/(segment_k_interp);

      std::vector<Manifold> Qs, Qs_tmp;

      for (const auto m : segments_control_points[s])
        Qs.emplace_back(*m);

      for (int i=0; i<degree-1; ++i)
      {
        for (int q=0; q<Qs.size()-1; ++q)
        {
          Qs_tmp.push_back( Qs[q].rplus(Qs[q+1].rminus(Qs[q]) * t_01) );
        }

        Qs = Qs_tmp;
        Qs_tmp.clear();
      }

      curve.push_back(Qs[0]);
    }
  }

  return curve;
}

} /* namespace manif */

#endif /* _MANIF_MANIF_DECASTELJAU_H_ */
