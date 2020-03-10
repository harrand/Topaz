#include "gl/tz_imgui/buffer_tracker.hpp"
#include "gl/object.hpp"
#include <string>
#include <cstring>

namespace tz::ext::imgui::gl
{
    BufferTracker::BufferTracker(tz::gl::Object* object): ImGuiWindow("Buffer Tracker"), object(object), tracked_buffer_id(std::nullopt){}

    void BufferTracker::track_buffer(std::size_t buffer_handle)
    {
        // Early out if we're already tracking it.
        if(this->tracked_buffer_id.has_value() && this->tracked_buffer_id.value() == buffer_handle)
            return;
        this->tracked_buffer_id = buffer_handle;
        this->reset();
    }

    void BufferTracker::target_object(tz::gl::Object* object)
    {
        this->object = object;
    }

    void BufferTracker::render()
    {
        auto shrink_as_necessary = [](int& a, int& b, std::size_t max, bool a_changed)
        {
            if(a + b >= max)
            {
                if(a_changed)
                    b = (max - a) - 1;
                else
                    a = (max - b) - 1;
            }
        };
        ImGui::Begin("Buffer Tracker");
        if(this->tracked_buffer_id.has_value())
        {
            std::size_t buf_id = this->tracked_buffer_id.value();
            tz::gl::IBuffer* buf = (*this->object)[buf_id];
            auto buf_size = buf->size();
            ImGui::Text("Size: %zu", buf_size);
            if(ImGui::SliderInt("View Byte Offset", &this->view_offset, 0, buf_size - 1))
            {
                shrink_as_necessary(this->view_offset, this->view_size, buf_size, true);
            }
            if(ImGui::SliderInt("View Byte Size", &this->view_size, 1, buf_size - this->view_offset - 1))
            {
                shrink_as_necessary(this->view_offset, this->view_size, buf_size, false);
            }
            // If mapped, we should just take the mapped data.
            std::vector<char> data;
            std::string data_string;
            data.resize(this->view_size);

            if(buf->is_mapped())
            {
                // will used cached value.
                auto blk = buf->map();
                std::memcpy(data.data(), reinterpret_cast<char*>(blk.begin) + this->view_offset, this->view_size);
            }
            else
            {
                buf->retrieve(this->view_offset, this->view_size, data.data());
            }

            ImGui::Text("Interpret data as:");
            static int t = 0;
            ImGui::RadioButton("Ints", &t, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Bytes", &t, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Floats", &t, 2);

            ImGui::Text("Data");
            switch(t)
            {
                case 0:
                {
                    // Ints
                    for(std::size_t i = 0; i < this->view_size; i += sizeof(int))
                    {
                        data_string += std::to_string(*reinterpret_cast<int*>(data.data() + i)) + " ";
                    }
                    break;
                }
                case 1:
                {
                    // Chars
                    for(char c : data)
                    {
                        data_string += std::to_string(static_cast<int>(c)) + " ";
                    }
                    break;
                }
                case 2:
                {
                    for(std::size_t i = 0; i < this->view_size; i += sizeof(float))
                    {
                        data_string += std::to_string(*reinterpret_cast<float*>(data.data() + i)) + " ";
                    }
                    break;
                }
            }
            ImGui::Text("%s", data_string.c_str());
            
        }
        else
        {
            ImGui::Text("Not currently tracking a buffer... Choose a buffer from the Object Tracker.");
        }
        ImGui::End();
    }

    void BufferTracker::reset()
    {
        this->view_offset = 0;
        this->view_size = 0;
    }
}