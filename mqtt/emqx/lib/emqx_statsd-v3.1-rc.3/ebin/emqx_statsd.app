{application,emqx_statsd,
             [{description,"Statsd for EMQ X"},
              {vsn, "v3.1-rc.3"},
              {modules, ['emqx_statsd','emqx_statsd_app','emqx_statsd_sup']},
              {registered,[emqx_statsd_sup]},
              {applications,[kernel,stdlib,prometheus]},
              {mod,{emqx_statsd_app,[]}}]}.
