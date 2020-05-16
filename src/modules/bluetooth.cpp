#include "modules/bluetooth.hpp"

#include <linux/rfkill.h>
#include <time.h>

#include "util/rfkill.hpp"

namespace waybar::modules {

Bluetooth::Bluetooth(const std::string& id, const Json::Value& config)
    : ALabel(config, "bluetooth", id, "{icon}", "{status}", 10),
      status_("disabled"),
      rfkill_{RFKILL_TYPE_BLUETOOTH} {
  thread_ = [this] {
    dp.emit();
    rfkill_.waitForEvent();
  };
  intervall_thread_ = [this] {
    auto now = std::chrono::system_clock::now();
    auto timeout = std::chrono::floor<std::chrono::seconds>(now + interval_);
    auto diff = std::chrono::seconds(timeout.time_since_epoch().count() % interval_.count());
    thread_.sleep_until(timeout - diff);
    dp.emit();
  };
}

auto Bluetooth::update(std::string format, fmt::dynamic_format_arg_store<fmt::format_context>& args) -> void {
  // Remove older status
  if (!status_.empty()) {
    label_.get_style_context()->remove_class(status_);
  }

  // Get status
  if (rfkill_.getState()) {
    status_ = "disabled";
  } else {
    status_ = "enabled";
  }

  // Add status class
  label_.get_style_context()->add_class(status_);

  // Add status and icon args
  args.push_back(status_);
  auto statusArg = fmt::arg("status", status_);
  args.push_back(std::cref(statusArg));
  auto icon = getIcon(0, status_);
  auto iconArg = fmt::arg("icon", icon);
  args.push_back(std::cref(iconArg));

  // Call parent update
  ALabel::update(format, args);
}

}  // namespace waybar::modules