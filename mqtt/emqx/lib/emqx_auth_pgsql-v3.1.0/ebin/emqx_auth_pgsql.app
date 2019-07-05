{application,emqx_auth_pgsql,
             [{description,"EMQ X Authentication/ACL with PostgreSQL"},
              {vsn, "v3.1.0"},
              {modules, ['emqx_acl_pgsql','emqx_auth_pgsql','emqx_auth_pgsql_app','emqx_auth_pgsql_cfg','emqx_auth_pgsql_cli','emqx_auth_pgsql_sup']},
              {registered,[emqx_auth_pgsql_sup]},
              {applications,[kernel,stdlib,epgsql,ecpool,clique,emqx_passwd]},
              {mod,{emqx_auth_pgsql_app,[]}}]}.
