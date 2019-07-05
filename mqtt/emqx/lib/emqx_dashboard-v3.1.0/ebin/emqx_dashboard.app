{application,emqx_dashboard,
             [{description,"EMQ X Web Dashboard"},
              {vsn, "v3.1.0"},
              {modules, ['emqx_dashboard','emqx_dashboard_admin','emqx_dashboard_api','emqx_dashboard_app','emqx_dashboard_cli','emqx_dashboard_sup']},
              {registered,[emqx_dashboard_sup]},
              {applications,[kernel,stdlib,mnesia,minirest]},
              {mod,{emqx_dashboard_app,[]}}]}.
