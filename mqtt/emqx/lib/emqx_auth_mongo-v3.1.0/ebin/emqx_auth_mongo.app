{application,emqx_auth_mongo,
             [{description,"EMQ X Authentication/ACL with MongoDB"},
              {vsn, "v3.1.0"},
              {modules, ['emqx_acl_mongo','emqx_auth_mongo','emqx_auth_mongo_app','emqx_auth_mongo_cfg','emqx_auth_mongo_sup']},
              {registered,[emqx_auth_mongo_sup]},
              {applications,[kernel,stdlib,mongodb,ecpool,clique,emqx_passwd]},
              {mod,{emqx_auth_mongo_app,[]}}]}.
