{application,emqx_plugin_template,
             [{description,"EMQ X Plugin Template"},
              {vsn, "v3.1.0"},
              {modules, ['emqx_cli_demo','emqx_plugin_template','emqx_plugin_template_app','emqx_plugin_template_sup']},
              {registered,[emqx_plugin_template_sup]},
              {applications,[kernel,stdlib]},
              {mod,{emqx_plugin_template_app,[]}}]}.
