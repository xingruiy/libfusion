#include "system.h"
#include "cuda_imgproc.h"

namespace fusion
{

System::~System()
{
    // relocalizer->should_quit = true;
    // relocalizer_thread.join();
}

System::System(IntrinsicMatrix base, const int NUM_PYR)
    : processed_frame_count(0), system_initialized(false)
{
    mapping = std::make_shared<DenseMapping>(base);
    odometry = std::make_shared<DenseOdometry>(base, NUM_PYR);
    extractor = std::make_shared<FeatureExtractor>();

    // relocalizer = std::make_shared<Relocalizer>(base);

    // relocalizer_thread = std::thread(&Relocalizer::main_loop, relocalizer.get());
}

void System::process_images(const cv::Mat depth, const cv::Mat image)
{
    std::chrono::system_clock clock;
    auto time_a = clock.now();

    cv::Mat depth_float;
    depth.convertTo(depth_float, CV_32FC1, 1 / 1000.f);

    cv::Mat desc;
    std::vector<cv::KeyPoint> points;
    extractor->image = image;
    std::thread ext(&FeatureExtractor::operator(), extractor.get());
    // std::thread ext(&FeatureExtractor::extract_features, extractor.get(), image, points, desc);

    current_frame = std::make_shared<RgbdFrame>(depth_float, image, processed_frame_count, 0);

    // needs initialization
    if (!system_initialized)
    {
        // break the loop
        last_tracked_frame = current_frame;
        create_new_keyframe();
        system_initialized = true;
    }

    odometry->track_frame(current_frame);

    if (!odometry->is_tracking_lost())
    {
        auto reference_image = odometry->get_reference_image();
        auto reference_frame = reference_image->get_reference_frame();

        mapping->update(reference_image->depth_float, reference_image->get_image(), reference_frame->get_pose());
        mapping->raycast(reference_image->vmap_pyr[0], reference_image->nmap_pyr[0], reference_frame->get_pose());

        // mapping->update(reference_image);
        // mapping->raycast(reference_image);
        reference_image->resize_device_map();
        reference_frame->set_scene_data(reference_image->get_vmap(), reference_image->get_nmap());

        if (keyframe_needed())
            create_new_keyframe();

        last_tracked_frame = current_frame;
        processed_frame_count += 1;
    }

    ext.join();
    auto time_b = clock.now();

    auto &key_points = extractor->key_points;
    auto &descritpors = extractor->descriptors;
    cv::Mat outImg;
    cv::drawKeypoints(image, key_points, outImg, cv::Scalar(255, 0, 0));
    cv::imshow("outImg", outImg);
    cv::waitKey(1);

    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(time_b - time_a).count() << std::endl;
}

bool System::keyframe_needed() const
{
    auto pose = current_frame->get_pose();
    auto ref_pose = current_keyframe->get_pose();
    if ((pose.inverse() * ref_pose).translation().norm() > 0.1f)
        return true;
    return false;
}

void System::create_new_keyframe()
{
    current_keyframe = last_tracked_frame;
    // relocalizer->insert_keyframe(current_keyframe);
    // mapping->create_new_submap();
}

cv::Mat System::get_rendered_scene() const
{
    return cv::Mat(odometry->get_reference_image()->get_rendered_image());
}

cv::Mat System::get_rendered_scene_textured() const
{
    return cv::Mat(odometry->get_reference_image()->get_rendered_scene_textured());
}

void System::restart()
{
    mapping->restart_mapping();
    odometry->restart_tracking();
    relocalizer->reset_relocalizer();
    system_initialized = false;
    processed_frame_count = 0;
}

void System::save_mesh_to_file(const char *str)
{
    // mapping->create_scene_mesh();
    // mapping->write_mesh_to_file(str);
}

void System::fetch_mesh_vertex_only(float3 *data, uint &max_size)
{
    // mapping->create_scene_mesh(data, max_size);
}

void System::fetch_mesh_with_normal(float3 *vertex, float3 *normal, uint &max_size)
{
    max_size = mapping->create_mesh_with_normal(vertex, normal);
}

void System::fetch_mesh_with_colour(float3 *vertex, uchar3 *colour, uint &max_size)
{
    // mapping->create_mesh_with_colour(vertex, colour, max_size);
}

void System::fetch_key_points(float *points, size_t &max_size)
{
    relocalizer->get_points(points, max_size);
}

void System::fetch_key_points_with_normal(float *points, float *normal, size_t &max_size)
{
    relocalizer->get_points_and_normal(points, normal, max_size);
}

bool System::is_initialized() const
{
    return this->system_initialized;
}

Eigen::Matrix4f System::get_current_camera_pose() const
{
    return odometry->get_current_pose_matrix();
}

} // namespace fusion