/*
 * @file:    ngx_http_user_define_check_module.c
 * @brief:   The simple module to check user-defined value.
 * @author:  YoungJoo.Kim <vozlt@vozlt.com>
 * @version:
 * @date:
 *
 * Compile:
 *           shell> ./configure --add-module=/path/to/nginx-module-udc
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#define NGX_USER_DEFINE_CHECK_DEFAULT_OUT_TYPE      "json"
#define NGX_USER_DEFINE_CHECK_DEFAULT_ALLOW_TEXT    "true"
#define NGX_USER_DEFINE_CHECK_DEFAULT_DENY_TEXT     "false"

typedef struct {
    ngx_str_t   agent_text;
} ngx_http_user_define_check_rule_t;

typedef struct {
    ngx_str_t       out_type;
    ngx_str_t       allow_text;
    ngx_str_t       deny_text;
    ngx_array_t     *rules;
} ngx_http_user_define_check_loc_conf_t;

static char *ngx_http_user_define_check_rule(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void *ngx_http_user_define_check_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_user_define_check_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
static char *ngx_http_set_user_define_check(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t ngx_http_user_define_check_commands[] = {

    /* on|off */
    { ngx_string("user_define_check"),
        NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS|NGX_CONF_TAKE1,
        ngx_http_set_user_define_check,
        0,
        0,
        NULL },

    /* agents */
    { ngx_string("user_define_check_agent"),
        NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_http_user_define_check_rule,
        NGX_HTTP_LOC_CONF_OFFSET,
        0,
        NULL },

    /* ouput type: json|text */
    { ngx_string("user_define_check_out_type"),
        NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_user_define_check_loc_conf_t, out_type),
        NULL },

    /* allow string */
    { ngx_string("user_define_check_allow_text"),
        NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_user_define_check_loc_conf_t, allow_text),
        NULL },

    /* deny string */
    { ngx_string("user_define_check_deny_text"),
        NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_conf_set_str_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_user_define_check_loc_conf_t, deny_text),
        NULL },

    ngx_null_command
};


static ngx_http_module_t ngx_http_user_define_check_module_ctx = {
    NULL,                                        /* preconfiguration */
    NULL,                                        /* postconfiguration */

    NULL,                                        /* create main configuration */
    NULL,                                        /* init main configuration */

    NULL,                                        /* create server configuration */
    NULL,                                        /* merge server configuration */

    ngx_http_user_define_check_create_loc_conf,  /* create location configuration */
    ngx_http_user_define_check_merge_loc_conf,   /* merge location configuration */
};


ngx_module_t ngx_http_user_define_check_module = {
    NGX_MODULE_V1,
    &ngx_http_user_define_check_module_ctx,      /* module context */
    ngx_http_user_define_check_commands,         /* module directives */
    NGX_HTTP_MODULE,                             /* module type */
    NULL,                                        /* init master */
    NULL,                                        /* init module */
    NULL,                                        /* init process */
    NULL,                                        /* init thread */
    NULL,                                        /* exit thread */
    NULL,                                        /* exit process */
    NULL,                                        /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_user_define_check_handler(ngx_http_request_t *r)
{
    size_t             size = 1024;
    ngx_int_t          rc;
    ngx_buf_t         *b;
    ngx_chain_t        out;

    ngx_uint_t                              i, fc;
    ngx_http_user_define_check_loc_conf_t   *alcf;
    ngx_http_user_define_check_rule_t       *rule;

    alcf = ngx_http_get_module_loc_conf(r, ngx_http_user_define_check_module);

    if (r->method != NGX_HTTP_GET && r->method != NGX_HTTP_HEAD) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    if (!r->args.len) {
        return NGX_HTTP_BAD_REQUEST;
    }

    rc = ngx_http_discard_request_body(r);

    if (rc != NGX_OK) {
        return rc;
    }

    ngx_str_set(&r->headers_out.content_type, "text/plain");

    if (r->method == NGX_HTTP_HEAD) {
        r->headers_out.status = NGX_HTTP_OK;

        rc = ngx_http_send_header(r);

        if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
            return rc;
        }
    }

    b = ngx_create_temp_buf(r->pool, size);
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    fc = 1;
    rule = alcf->rules->elts;
    for (i = 0; i < alcf->rules->nelts; i++) {
        if (ngx_strncasecmp(rule[i].agent_text.data, (u_char *) r->args.data,
                    rule[i].agent_text.len) == 0
                    && rule[i].agent_text.len == r->args.len) {
            fc = 0;
            break;
        }
    }
    /* true */
    if (!fc) {
        if (ngx_strcmp(alcf->out_type.data, "json") == 0) {
            b->last = ngx_sprintf(b->last, "{\"status\":\"%s\"}", alcf->allow_text.data);
        } else {
            b->last = ngx_sprintf(b->last, "%s", alcf->allow_text.data);
        }
    }
    /* false */
    else {
        if (ngx_strcmp(alcf->out_type.data, "json") == 0) {
            b->last = ngx_sprintf(b->last, "{\"status\":\"%s\"}", alcf->deny_text.data);
        } else {
            b->last = ngx_sprintf(b->last, "%s", alcf->deny_text.data);
        }
    }

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = b->last - b->pos;

    b->last_buf = (r == r->main) ? 1 : 0;
    b->last_in_chain = 1;

    out.buf = b;
    out.next = NULL;

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    return ngx_http_output_filter(r, &out);
}


static char *
ngx_http_user_define_check_rule(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_user_define_check_loc_conf_t *alcf = conf;

    ngx_str_t                           *value;
    ngx_http_user_define_check_rule_t   *rule;

    value = cf->args->elts;

    if (alcf->rules == NULL) {
        alcf->rules = ngx_array_create(cf->pool, 1, sizeof(ngx_http_user_define_check_rule_t));

        if (alcf->rules == NULL) {
            return NGX_CONF_ERROR;
        }
    }

    rule = ngx_array_push(alcf->rules);
    if (rule == NULL) {
        return NGX_CONF_ERROR;
    }

    rule->agent_text.len = value[1].len;
    rule->agent_text.data = value[1].data;

    return NGX_CONF_OK;
}


static void *
ngx_http_user_define_check_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_user_define_check_loc_conf_t *conf;
    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_user_define_check_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    return conf;
}


static char *
ngx_http_user_define_check_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{   
    ngx_http_user_define_check_loc_conf_t *prev = parent;
    ngx_http_user_define_check_loc_conf_t *conf = child;

    if (conf->rules == NULL) {
        conf->rules = prev->rules;
    }

    if (conf->out_type.data == NULL) {
        conf->out_type.len = sizeof(NGX_USER_DEFINE_CHECK_DEFAULT_OUT_TYPE);
        conf->out_type.data = (u_char *) NGX_USER_DEFINE_CHECK_DEFAULT_OUT_TYPE;
    }

    if (conf->allow_text.data == NULL) {
        conf->allow_text.len = sizeof(NGX_USER_DEFINE_CHECK_DEFAULT_ALLOW_TEXT);
        conf->allow_text.data = (u_char *) NGX_USER_DEFINE_CHECK_DEFAULT_ALLOW_TEXT;
    }

    if (conf->deny_text.data == NULL) {
        conf->deny_text.len = sizeof(NGX_USER_DEFINE_CHECK_DEFAULT_DENY_TEXT);
        conf->deny_text.data = (u_char *) NGX_USER_DEFINE_CHECK_DEFAULT_DENY_TEXT;
    }

    return NGX_CONF_OK;
}


static char *
ngx_http_set_user_define_check(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_user_define_check_handler;

    return NGX_CONF_OK;
}
