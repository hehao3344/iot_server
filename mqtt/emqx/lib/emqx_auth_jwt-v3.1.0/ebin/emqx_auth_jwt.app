{application,emqx_auth_jwt,
             [{description,"EMQ X Authentication with JWT"},
              {vsn, "v3.1.0"},
              {modules, ['emqx_auth_jwt','emqx_auth_jwt_app','emqx_auth_jwt_cfg']},
              {registered,[emqx_auth_jwt_sup]},
              {applications,[kernel,stdlib,jwerl,clique]},
              {mod,{emqx_auth_jwt_app,[]}}]}.