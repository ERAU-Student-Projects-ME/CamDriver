#include <memory>
#include <vector>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/compressed_image.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/imgcodecs.hpp>

class ImageCompressorNode : public rclcpp::Node
{
public:
  ImageCompressorNode() : Node("image_compressor_node")
  {
    // Define SensorDataQoS (Best Effort durability/reliability, small queue depth)
    auto sensor_qos = rclcpp::SensorDataQoS();

    // Subscriber with SensorDataQoS
    image_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
      "/image_raw",
      sensor_qos,
      std::bind(&ImageCompressorNode::imageCallback, this, std::placeholders::_1)
    );

    // Compressed Image Publisher
    compressed_pub_ = this->create_publisher<sensor_msgs::msg::CompressedImage>(
      "/image_raw/compressed",
      sensor_qos
    );

    RCLCPP_INFO(this->get_logger(), "Image Compressor Node initialized.");
  }

private:
  void imageCallback(const sensor_msgs::msg::Image::ConstSharedPtr msg)
  {
    try {
      // Convert ROS Image message to OpenCV Mat
      cv_bridge::CvImageConstPtr cv_ptr = cv_bridge::toCvShare(msg, msg->encoding);

      // Create output CompressedImage message
      auto compressed_msg = std::make_shared<sensor_msgs::msg::CompressedImage>();
      compressed_msg->header = msg->header;
      compressed_msg->format = "jpeg";

      // Encode image to JPEG
      std::vector<uchar> buffer;
      std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 15}; // 15% JPEG quality
      cv::imencode(".jpg", cv_ptr->image, buffer, params);

      compressed_msg->data = buffer;

      // Publish compressed image
      compressed_pub_->publish(*compressed_msg);
    }
    catch (const cv_bridge::Exception& e) {
      RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
    }
  }

  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;
  rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr compressed_pub_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ImageCompressorNode>());
  rclcpp::shutdown();
  return 0;
}
