#include <gst/gst.h>
#include <gst/gstmeta.h>
#include <iostream>
#include <sstream>

static int frame_counter = 0;

// Define a custom meta type
typedef struct {
    GstMeta meta;
    int counter;
} GstFrameCounterMeta;

// Init function for GstCounterMeta
static gboolean gst_counter_meta_init(GstMeta* meta, gpointer params, GstBuffer* buffer) {
    auto counter_meta = reinterpret_cast<GstFrameCounterMeta*>(meta);
    counter_meta->counter = 0; // Initialize the counter to 0
    return TRUE;
}

const GstMetaInfo* gst_counter_meta_get_info() {
    static const GstMetaInfo* meta_info = nullptr;
    if (g_once_init_enter(&meta_info)) {
        const GstMetaInfo* new_info = gst_meta_register(
            g_quark_from_static_string("GstCounterMeta"),
            "GstCounterMeta",
            sizeof(GstFrameCounterMeta),
            gst_counter_meta_init,
            nullptr,
            nullptr
        );
        g_once_init_leave(&meta_info, new_info);
    }
    return meta_info;
}

GstFrameCounterMeta* gst_buffer_add_counter_meta(GstBuffer* buffer, const int counter) {
    auto* meta = reinterpret_cast<GstFrameCounterMeta*>(gst_buffer_add_meta(
        buffer, gst_counter_meta_get_info(), nullptr));
    if (meta) {
        meta->counter = counter;
    }
    return meta;
}

// Callback function for identity element to embed a counter
static void on_handoff_embed(GstElement* identity, GstBuffer* buffer, gpointer user_data) {
    auto* text_overlay = static_cast<GstElement*>(user_data);

    std::ostringstream text;
    text << "Frame Counter: " << frame_counter;

    g_object_set(text_overlay, "text", text.str().c_str(), nullptr);

    gst_buffer_add_counter_meta(buffer, frame_counter);
    std::cout << "Embedded frame counter: " << frame_counter << std::endl;
    frame_counter++;
}

// Callback function for identity element to extract a counter
static void on_handoff_extract(GstElement* identity, GstBuffer* buffer, gpointer user_data) {
    gpointer state = nullptr;
    GstMeta* meta;
    while ((meta = gst_buffer_iterate_meta(buffer, &state))) {
        if (meta->info == gst_counter_meta_get_info()) {
            auto counter_meta = reinterpret_cast<GstFrameCounterMeta*>(meta);
            std::cout << "Extracted frame counter: " << counter_meta->counter << std::endl;
        }
    }
}

int main([[maybe_unused]] const int argc, [[maybe_unused]] char* argv[]) {
    gst_init(nullptr, nullptr);

    auto pipeline = gst_parse_launch(
        "videotestsrc ! identity name=embed_identity ! textoverlay name=text_overlay ! identity name=extract_identity ! autovideosink",
        nullptr);

    if (!pipeline) {
        g_printerr("Failed to create pipeline\n");
        return -1;
    }

    auto embed_identity = gst_bin_get_by_name(GST_BIN(pipeline), "embed_identity");
    auto text_overlay = gst_bin_get_by_name(GST_BIN(pipeline), "text_overlay");
    auto extract_identity = gst_bin_get_by_name(GST_BIN(pipeline), "extract_identity");

    if (!embed_identity || !text_overlay || !extract_identity) {
        g_printerr("Failed to get elements\n");
        return -1;
    }

    // Connect the signal to embed the counter using the textoverlay element and extract counters
    g_signal_connect(embed_identity, "handoff", G_CALLBACK(on_handoff_embed), text_overlay);
    g_signal_connect(extract_identity, "handoff", G_CALLBACK(on_handoff_extract), nullptr);

    // Set the pipeline to playing state
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Run the pipeline
    auto bus = gst_element_get_bus(pipeline);
    GstMessage* msg = nullptr;
    bool terminate = false;
    while (!terminate) {
        msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                         static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
        if (msg != nullptr) {
            switch (GST_MESSAGE_TYPE(msg)) {
                case GST_MESSAGE_ERROR: {
                    GError* err;
                    gchar* debug_info;
                    gst_message_parse_error(msg, &err, &debug_info);
                    g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
                    g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
                    g_clear_error(&err);
                    g_free(debug_info);
                    terminate = true;
                    break;
                }
                case GST_MESSAGE_EOS:
                    g_print("End-Of-Stream reached.\n");
                    terminate = true;
                    break;
                default:
                    g_printerr("Unexpected message received.\n");
                    break;
            }
            gst_message_unref(msg);
        }
    }

    // Free resources
    gst_object_unref(embed_identity);
    gst_object_unref(extract_identity);
    gst_object_unref(text_overlay);
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}
