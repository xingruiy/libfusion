#ifndef __DEVICE_MAP__
#define __DEVICE_MAP__

#include "map_struct.h"
#include "intrinsic_matrix.h"
#include <sophus/se3.hpp>
#include <opencv2/cudaarithm.hpp>
#include <thrust/device_vector.h>

namespace fusion
{
namespace cuda
{

void update(
    MapStruct<true> map_struct,
    const cv::cuda::GpuMat depth,
    const cv::cuda::GpuMat image,
    const Sophus::SE3d &frame_pose,
    const IntrinsicMatrix K,
    cv::cuda::GpuMat &cv_flag,
    cv::cuda::GpuMat &cv_pos_array,
    HashEntry *visible_blocks,
    uint &visible_block_count);

void update_weighted(
    MapStruct<true> map_struct,
    const cv::cuda::GpuMat depth,
    const cv::cuda::GpuMat normal,
    const cv::cuda::GpuMat image,
    const Sophus::SE3d &frame_pose,
    const IntrinsicMatrix K,
    cv::cuda::GpuMat &cv_flag,
    cv::cuda::GpuMat &cv_pos_array,
    HashEntry *visible_blocks,
    uint &visible_block_count);

void create_rendering_blocks(
    MapStruct<true> map_struct,
    uint count_visible_block,
    uint &count_redering_block,
    HashEntry *visible_blocks,
    cv::cuda::GpuMat &zrange_x,
    cv::cuda::GpuMat &zrange_y,
    RenderingBlock *rendering_blocks,
    const Sophus::SE3d &frame_pose,
    const IntrinsicMatrix cam_params);

void raycast(
    MapStruct<true> map_struct,
    cv::cuda::GpuMat vmap,
    cv::cuda::GpuMat nmap,
    cv::cuda::GpuMat zrange_x,
    cv::cuda::GpuMat zrange_y,
    const Sophus::SE3d &pose,
    const IntrinsicMatrix intrinsic_matrix);

void raycast_with_colour(
    MapStruct<true> map_struct,
    cv::cuda::GpuMat vmap,
    cv::cuda::GpuMat nmap,
    cv::cuda::GpuMat image,
    cv::cuda::GpuMat zrange_x,
    cv::cuda::GpuMat zrange_y,
    const Sophus::SE3d &pose,
    const IntrinsicMatrix intrinsic_matrix);

void create_mesh_vertex_only(
    MapStruct<true> map_struct,
    uint &block_count,
    HashEntry *block_list,
    uint &triangle_count,
    float3 *vertex_data);

void create_mesh_with_normal(
    MapStruct<true> map_struct,
    uint &block_count,
    HashEntry *block_list,
    uint &triangle_count,
    float3 *vertex_data,
    float3 *vertex_normal);

void create_mesh_with_colour(
    MapStruct<true> map_struct,
    uint &block_count,
    HashEntry *block_list,
    uint &triangle_count,
    float3 *vertex_data,
    uchar3 *vertex_colour);

} // namespace cuda
} // namespace fusion

#endif