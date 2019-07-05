{application,emqx_auth_redis,
             [{description,"EMQ X Authentication/ACL with Redis"},
              {vsn, "v3.1.0"},
              {modules, ['emqx_acl_redis','emqx_auth_redis','emqx_auth_redis_app','emqx_auth_redis_cfg','emqx_auth_redis_cli','emqx_auth_redis_sup']},
              {registered,[emqx_auth_redis_sup]},
              {applications,[kernel,stdlib,eredis,eredis_cluster,ecpool,
                             clique,emqx_passwd]},
              {mod,{emqx_auth_redis_app,[]}}]}.
