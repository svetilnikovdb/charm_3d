#include "charm_globals.h"
#include "charm_amr.h"

void charm_init_context_yaml(charm_ctx_t *ctx);

charm_ctx_t           ctx;
p4est_t              *p4est;

int main (int argc, char **argv)
{
    int                   mpiret;
    sc_MPI_Comm           mpicomm;
    p4est_connectivity_t *conn;
    char                  charm_str[128];

    sc_set_log_defaults(NULL, NULL, CHARM_LOG_LEVEL);

    mpiret = sc_MPI_Init (&argc, &argv);
    SC_CHECK_MPI (mpiret);
    mpicomm = sc_MPI_COMM_WORLD;

    sprintf(charm_str, "\nCHARM_3D, v.%s\n\n", CHARM_VERSION);
    CHARM_GLOBAL_ESSENTIAL(charm_str);

    charm_package_id = sc_package_register(NULL, CHARM_LOG_LEVEL, CHARM_STRING,
            "Chemistry on AMR");

    charm_init_context_yaml(&ctx);

    conn = charm_conn_create(&ctx);
    if (!conn) {
        charm_abort(NULL, 1);
    }
    CHARM_ASSERT(p4est_connectivity_is_valid(conn));

    p4est = p4est_new_ext (mpicomm, conn,  0, ctx.min_level, 1, sizeof (charm_data_t),
            charm_init_initial_condition, (void *) (&ctx));
    charm_set_p4est(p4est);
    charm_write_solution (p4est);
    charm_adapt_init(p4est);
    charm_timesteps (p4est);
    p4est_destroy (p4est);
    p4est_connectivity_destroy (conn);
    sc_finalize ();
    mpiret = sc_MPI_Finalize ();
    SC_CHECK_MPI(mpiret);
    return 0;
}
