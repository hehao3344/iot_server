{application,emqx_reloader,
             [{description,"EMQ X Reloader Plugin"},
              {vsn, "v3.1-rc.2"},
              {modules, ['emqx_reloader','emqx_reloader_app','emqx_reloader_cli','emqx_reloader_config','emqx_reloader_sup']},
              {registered,[emqx_reloader_sup]},
              {applications,[kernel,stdlib]},
              {mod,{emqx_reloader_app,[]}}]}.
