//
// Created by zhrv on 26.10.17.
//

#include "charm_bnd_cond.h"

static charm_mat_t *glob_mat;

charm_bnd_types_t charm_bnd_type_by_name(const char* name) {
    int i = 0;
    while (charm_bnd_types[i] != NULL) {
        if (strcmp(charm_bnd_types[i], name) == 0) {
            return (charm_bnd_types_t)i;
        }
        i++;
    }
    return BOUND_UNKNOWN;
}


void charm_bnd_cond(p4est_t* p4est, p4est_topidx_t treeid, int8_t face,
                    charm_prim_t *par_in, charm_prim_t *par_out, charm_real_t n[CHARM_DIM])
{
    charm_ctx_t *ctx = charm_get_ctx(p4est);
    charm_tree_attr_t *attr = charm_get_tree_attr(p4est, treeid);
    charm_bnd_t *bnd = attr->bnd[face];
    glob_mat = charm_mat_find_by_id(ctx, par_in->mat_id);
    CHARM_ASSERT(bnd);
    bnd->bnd_fn(par_in, par_out, face, bnd->params, n);
    par_out->mat_id = par_in->mat_id;
    glob_mat->eos_fn(p4est, par_out, 3); // (T,p) => (r, cz, e)
    par_out->e_tot = par_out->e+0.5*_MAG_(par_out->u, par_out->v, par_out->w);
}


void charm_bnd_cond_fn_inlet(charm_prim_t *par_in, charm_prim_t *par_out, int8_t face, charm_real_t* param, charm_real_t n[CHARM_DIM])
{
    CHARM_ASSERT(param);

    charm_prim_cpy(par_out, par_in); //@todo

    par_out->u = param[0];
    par_out->v = param[1];
    par_out->w = param[2];
    par_out->t = param[3];
    par_out->p = param[4];
}


void charm_bnd_cond_fn_outlet(charm_prim_t *par_in, charm_prim_t *par_out, int8_t face, charm_real_t* param, charm_real_t n[CHARM_DIM])
{
    charm_prim_cpy(par_out, par_in);
}


void charm_bnd_cond_fn_wall_slip(charm_prim_t *par_in, charm_prim_t *par_out, int8_t face, charm_real_t* param, charm_real_t n[CHARM_DIM])
{
    int i;
    charm_real_t  v[3] = {par_in->u, par_in->v, par_in->w};

    charm_prim_cpy(par_out, par_in);

    charm_real_t   svn = scalar_prod( v, n );
    charm_real_t   vv[3] = {n[0]*svn, n[1]*svn, n[2]*svn};
    for (i = 0; i < 3; i++) {
        v[i] -= vv[i];
        v[i] -= vv[i];
    }
    par_out->u = v[0];
    par_out->v = v[1];
    par_out->w = v[2];
}


// @todo
void charm_bnd_cond_fn_wall_no_slip(charm_prim_t *par_in, charm_prim_t *par_out, int8_t face, charm_real_t* param, charm_real_t n[CHARM_DIM])
{
    int i;
    charm_real_t  v[3] = {par_in->u, par_in->v, par_in->w};

    charm_prim_cpy(par_out, par_in);

    charm_real_t   svn = scalar_prod( v, n );
    charm_real_t   vv[3] = {n[0]*svn, n[1]*svn, n[2]*svn};
    for (i = 0; i < 3; i++) {
        v[i] -= vv[i];
        // v[i] -= vv[i];
    }
    par_out->u = v[0];
    par_out->v = v[1];
    par_out->w = v[2];
    par_out->t = param[0];
}


void charm_bnd_cond_fn_mass_flow(charm_prim_t *par_in, charm_prim_t *par_out, int8_t face, charm_real_t* param, charm_real_t n[CHARM_DIM])
{
    p4est_t *p4est = charm_get_p4est();

    charm_int_t c_count = charm_get_comp_count(p4est);
    charm_int_t ic;
    charm_real_t vel, v2, k;
    charm_real_t _m      = param[0];	// заданный расход
    charm_real_t _p_stat = param[1];	// статическое давление (сверхзвук)
    charm_real_t _t_tot  = param[2];	// полная температура
    charm_real_t _cos_x  = param[3];
    charm_real_t _cos_y  = param[4];	// косинусы угла входа потока
    charm_real_t _cos_z  = param[5];

    charm_real_t v[CHARM_DIM] = {_cos_x, _cos_y, _cos_z};

    charm_real_t flow_dir = scalar_prod(n, v);

    charm_prim_cpy(par_out, par_in);

    if (flow_dir > 0.) {
        vel = par_out->u * n[0] + par_out->v * n[1] + par_out->w * n[2];

        if (vel < par_out->cz)
        {
            par_out->p = param[6];
        } else {
            param[6] = par_out->p;
        }
    }
    else {
        for (ic=0; ic < c_count; ic++)
            par_out->c[ic] = param[7+ic];

        glob_mat->eos_fn(p4est, par_out, 0);  // вычислили Cp, Cv, Gama

        v2 = par_out->u*par_out->u+par_out->v*par_out->v+par_out->w*par_out->w;
        if ( v2 > par_out->cz*par_out->cz) par_out->p = _p_stat; // если втекает и свехзвук

        k = pow( _m * ( par_out->cp - par_out->cv ) / ( par_out->p * flow_dir ) , 2. ) / ( 2. * par_out->cp);

        par_out->t = ( - 1 + sqrt( 1.0 + 4.0 * k * _t_tot ) ) / ( 2. * k );

        vel = fabs( _m / ( par_out->r * flow_dir ) );
        par_out->u = _cos_x * vel;
        par_out->v = _cos_y * vel;
        par_out->w = _cos_z * vel;
    }


}
