{application,emqx_auth_clientid,
             [{description,"EMQ X Authentication with ClientId/Password"},
              {vsn, "v3.1.0"},
              {modules, ['emqx_auth_clientid','emqx_auth_clientid_api','emqx_auth_clientid_app','emqx_auth_clientid_cfg']},
              {registered,[emqx_auth_clientid_sup]},
              {applications,[kernel,stdlib,clique]},
              {mod,{emqx_auth_clientid_app,[]}}]}.
