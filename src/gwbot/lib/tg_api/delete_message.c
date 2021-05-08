
#include <gwbot/base.h>
#include <gwbot/lib/tg_api.h>
#include <gwbot/lib/tg_api/delete_message.h>

int tga_delete_message(tga_handle_t *handle, const tga_delete_message_t *ctx)
{
        char buf[256];

        snprintf(buf, sizeof(buf),
                "chat_id=%" PRId64
                "&message_id=%" PRIu64,
                ctx->chat_id,
                ctx->message_id);

        tga_set_body(handle, buf);
        tga_set_method(handle, "deleteMessage");
        return tg_api_post(handle);
}
