// kaoru/src/llm_bridge.c
#include "llm_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>

/* ---------- libcurl write callback ---------- */
static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total = size * nmemb;
    char **buf = (char **)userdata;
    size_t old_len = *buf ? strlen(*buf) : 0;
    char *new_buf = realloc(*buf, old_len + total + 1);
    if (!new_buf) return 0;
    memcpy(new_buf + old_len, ptr, total);
    new_buf[old_len + total] = '\0';
    *buf = new_buf;
    return total;
}

/* ---------- Init / Free ---------- */
int kaoru_llm_init(KaoruLLMBridge *b, const char *server_url, int n_ctx) {
    if (!b || !server_url) return -1;
    memset(b, 0, sizeof(*b));
    b->server_url = strdup(server_url);
    b->n_ctx      = n_ctx;
    b->loaded     = 1;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    return 0;
}

void kaoru_llm_free(KaoruLLMBridge *b) {
    if (!b) return;
    free(b->server_url);
    curl_global_cleanup();
    memset(b, 0, sizeof(*b));
}

/* ---------- Mental state → sampling params ---------- */
SamplingParams map_mental_state_to_params(const KaoruMentalState *m) {
    SamplingParams p;
    p.temperature       = 0.9f - (m->logic_focus * 0.6f);
    p.top_p             = 0.95f - (m->logic_focus * 0.25f);
    p.presence_penalty  = 0.1f + (m->relational_distance * 0.4f);
    p.frequency_penalty = 0.1f + (m->relational_distance * 0.3f);
    p.max_tokens        = (int)(128.0f + m->agency_drive * 384.0f);
    p.seed              = -1;
    if (p.temperature < 0.05f) p.temperature = 0.05f;
    if (p.top_p < 0.10f)       p.top_p = 0.10f;
    if (p.max_tokens < 32)     p.max_tokens = 32;
    return p;
}

/* ---------- JSON escape (minimal) ---------- */
static void json_escape(const char *src, char *dst, size_t dst_len) {
    size_t j = 0;
    for (size_t i = 0; src[i] && j + 6 < dst_len; i++) {
        unsigned char c = (unsigned char)src[i];
        if (c == '"' || c == '\\') { dst[j++] = '\\'; dst[j++] = (char)c; }
        else if (c == '\n') { dst[j++] = '\\'; dst[j++] = 'n'; }
        else if (c == '\r') { dst[j++] = '\\'; dst[j++] = 'r'; }
        else if (c == '\t') { dst[j++] = '\\'; dst[j++] = 't'; }
        else if (c < 0x20) { /* skip control chars */ }
        else { dst[j++] = (char)c; }
    }
    dst[j] = '\0';
}

/* ---------- Extract "content":"..." from JSON response ---------- */
/* ใช้ string search ง่ายๆ — ไม่ parse JSON เต็ม (พอสำหรับ test) */
static int extract_content(const char *json, char *out, size_t out_len) {
    const char *key = "\"content\":\"";
    const char *p = strstr(json, key);
    if (!p) return -1;
    p += strlen(key);

    size_t j = 0;
    while (*p && j + 1 < out_len) {
        if (*p == '\\' && *(p + 1)) {
            char esc = *(p + 1);
            if (esc == 'n') out[j++] = '\n';
            else if (esc == 't') out[j++] = '\t';
            else if (esc == 'r') out[j++] = '\r';
            else if (esc == '"') out[j++] = '"';
            else if (esc == '\\') out[j++] = '\\';
            else out[j++] = esc;
            p += 2;
        } else if (*p == '"') {
            break;
        } else {
            out[j++] = *p++;
        }
    }
    out[j] = '\0';
    return (int)j;
}

/* ---------- Main generate ---------- */
int kaoru_generate_raw(KaoruLLMBridge *b, const char *system_prompt,
                       const char *user_prompt, SamplingParams p,
                       char *out, size_t out_len) {
    if (!b || !b->loaded || !out || out_len == 0) return -1;

    /* Random seed ถ้าไม่ระบุ */
    if (p.seed < 0) p.seed = (int)(time(NULL) & 0x7FFFFFFF);

    /* Escape prompts */
    char sys_esc[2048], usr_esc[2048];
    json_escape(system_prompt ? system_prompt : "You are Kaoru.",
                sys_esc, sizeof(sys_esc));
    json_escape(user_prompt ? user_prompt : "", usr_esc, sizeof(usr_esc));

    /* Build JSON body */
    char json[8192];
    int n = snprintf(json, sizeof(json),
        "{"
        "\"messages\":["
          "{\"role\":\"system\",\"content\":\"%s\"},"
          "{\"role\":\"user\",\"content\":\"%s\"}"
        "],"
        "\"temperature\":%.3f,"
        "\"top_p\":%.3f,"
        "\"presence_penalty\":%.3f,"
        "\"frequency_penalty\":%.3f,"
        "\"max_tokens\":%d,"
        "\"seed\":%d,"
        "\"stream\":false"
        "}",
        sys_esc, usr_esc,
        p.temperature, p.top_p, p.presence_penalty, p.frequency_penalty,
        p.max_tokens, p.seed);
    if (n <= 0 || n >= (int)sizeof(json)) return -1;

    /* HTTP POST */
    CURL *curl = curl_easy_init();
    if (!curl) return -1;

    char url[512];
    snprintf(url, sizeof(url), "%s/v1/chat/completions", b->server_url);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    char *resp = NULL;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);

    CURLcode rc = curl_easy_perform(curl);

    int result = -1;
    if (rc == CURLE_OK && resp) {
        result = extract_content(resp, out, out_len);
    } else if (resp) {
        fprintf(stderr, "[llm_bridge] HTTP error: %s\n", curl_easy_strerror(rc));
        fprintf(stderr, "[llm_bridge] Response: %.500s\n", resp);
    }

    free(resp);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return result;
}