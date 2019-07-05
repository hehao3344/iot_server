{application,emqx_stomp,
             [{description,"EMQ X Stomp Protocol Plugin"},
              {vsn, "v3.1-rc.2"},
              {modules, ['emqx_stomp','emqx_stomp_config','emqx_stomp_connection','emqx_stomp_frame','emqx_stomp_heartbeat','emqx_stomp_protocol','emqx_stomp_transaction']},
              {registered,[emqx_stomp_sup]},
              {applications,[kernel,stdlib,clique]},
              {mod,{emqx_stomp,[]}}]}.
