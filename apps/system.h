#ifndef SYSTEM_H
#define SYSTEM_H

#include "rgbd_frame.h"
#include "dense_mapping.h"
#include "dense_odometry.h"
#include "feature_graph.h"
#include <thread>
#include <eigen3/Eigen/Core>
#include <opencv2/opencv.hpp>
#include <xfusion/core/intrinsic_matrix.h>

namespace fusion
{

class System
{
public:
    ~System();
    System(IntrinsicMatrix base, const int NUM_PYR);
    void process_images(const cv::Mat depth, const cv::Mat image);

    // get rendered ray tracing map
    cv::Mat get_shaded_depth();
    cv::Mat get_rendered_scene() const;
    cv::Mat get_rendered_scene_textured() const;

    // create a mesh from the map
    // and save it to a named file
    // it only contains vertex data
    void save_mesh_to_file(const char *str);

    // create mesh and store in the address
    // users are reponsible for allocating
    // the adresses in CUDA using `cudaMalloc`
    size_t fetch_mesh_vertex_only(float *vertex);
    size_t fetch_mesh_with_normal(float *vertex, float *normal);
    size_t fetch_mesh_with_colour(float *vertex, unsigned char *colour);

    // key points
    void fetch_key_points(float *points, size_t &max_size);
    void fetch_key_points_with_normal(float *points, float *normal, size_t &max_size);

    bool is_initialized;

    // system controls
    void change_colour_mode(int colour_mode = 0);
    void change_run_mode(int run_mode = 0);
    void restart();

    void writeMapToDisk(std::string file_name) const;
    void readMapFromDisk(std::string file_name);

    Eigen::Matrix4f get_camera_pose() const;

private:
    RgbdFramePtr current_frame;
    RgbdFramePtr last_tracked_frame;
    RgbdFramePtr current_keyframe;

    size_t frame_id;

    // System modules
    std::shared_ptr<DenseMapping> mapping;
    std::shared_ptr<DenseOdometry> odometry;
    std::shared_ptr<FeatureGraph> features;
    std::shared_ptr<RgbdOdometry> odom;
    std::thread feature_thread;

    // Return TRUE if a new key frame is desired
    // return FALSE otherwise
    // TODO: this needs to be redesigned.
    bool keyframe_needed() const;
    void create_keyframe();
    void initialization();

    cv::cuda::GpuMat device_depth_float;
    cv::cuda::GpuMat device_image_uchar;
    cv::cuda::GpuMat device_vmap_cast;
    cv::cuda::GpuMat device_nmap_cast;
};

} // namespace fusion

#endif