{application,emqx_retainer,
             [{description,"EMQ X Retainer"},
              {vsn, "v3.1.1"},
              {modules, ['emqx_retainer','emqx_retainer_app','emqx_retainer_cfg','emqx_retainer_cli','emqx_retainer_sup']},
              {registered,[emqx_retainer_sup]},
              {applications,[kernel,stdlib,clique]},
              {mod,{emqx_retainer_app,[]}}]}.
