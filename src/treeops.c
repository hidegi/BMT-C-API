/****************************************************************************
 * Copyright (c) 2024 Hidegi
 *
 * This software is provided ‘as-is’, without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ****************************************************************************/
#include "internal.h"
#define BMT_MAX(a, b) ((a) > (b) ? (a) : (b))

static BMT_node _bmt_rotate_left(BMT_node n, BMT_node* head);
static BMT_node _bmt_rotate_right(BMT_node n, BMT_node* head);
static BMT_bool _bmt_transplant_rbt(BMT_node x, BMT_node w, BMT_node* head);
static BMT_bool _bmt_transplant_proc_0(BMT_node x);
static BMT_bool _bmt_transplant_proc_1(BMT_node x, BMT_node w, BMT_node* head);
static BMT_bool _bmt_transplant_proc_2(BMT_node x, BMT_node w, BMT_node* head);
static BMT_bool _bmt_transplant_proc_3(BMT_node x, BMT_node w, BMT_node* head);
static BMT_bool _bmt_transplant_proc_4(BMT_node x, BMT_node w, BMT_node* head);

static BMT_node _bmt_find_max(BMT_node n)
{
    return n ? (!n->major ? n : _bmt_find_max(n->major)) : NULL;
}

static BMT_node _bmt_find_min(BMT_node n)
{
    return n ? (!n->minor ? n : _bmt_find_min(n->minor)) : NULL;
}

BMT_bool _bmt_is_major(const BMT_node node)
{
    return (node && node->parent) ? (node->parent->major == node) : BMT_FALSE;
}

BMT_bool _bmt_is_root(const BMT_node node)
{
    return (node && !node->parent);
}

static BMT_node _bmt_find_member(const BMT_node node)
{
    if (!node || !node->parent || !node->parent->parent)
        return NULL;
    return _bmt_is_major(node->parent) ? node->parent->parent->minor : node->parent->parent->major;
}

static BMT_size _bmt_calculate_black_depth(const BMT_node rbt)
{
    if (!rbt)
        return 1;

    BMT_size count = 0;

    if (!rbt->red)
        count++;

    BMT_size majorDepth = _bmt_calculate_black_depth(rbt->major);
    BMT_size minorDepth = _bmt_calculate_black_depth(rbt->minor);
    return count + BMT_MAX(minorDepth, majorDepth);
}

static BMT_bool _bmt_verify_rbt_impl(const BMT_node rbt, BMT_size depth, BMT_size ref)
{
    if (rbt)
    {
        if (rbt->red)
        {
            if (!rbt->parent)
                return BMT_FALSE;
            else
            {
                if (rbt->parent->red)
                    return BMT_FALSE;
            }
        }
        else
        {
            ++depth;
        }

        if (rbt->major || rbt->minor)
        {
            return _bmt_verify_rbt_impl(rbt->major, depth, ref) && _bmt_verify_rbt_impl(rbt->minor, depth, ref);
        }
        else
        {
            return !rbt->red ? (ref == depth) : BMT_TRUE;
        }
    }
    return ref == depth;
}

BMT_bool bmt_IsValidRBT(const BMT_node rbt)
{
    if (!bmt_IsTree(rbt))
        return BMT_FALSE;

    BMT_size depth = _bmt_calculate_black_depth(rbt) - 1;
    return _bmt_verify_rbt_impl(rbt, 0, depth);
}

void _bmt_fix_rbt_violations(BMT_node node, BMT_node* head)
{
    if (node)
    {
        if (node->red && node->parent->red)
        {
            BMT_node m = _bmt_find_member(node);
            BMT_node p = node->parent;
            BMT_node g = p->parent;
            BMT_node r = NULL;

            BMT_bool isRed = m != NULL ? m->red : BMT_FALSE;

            if (isRed)
            {
                // color switch
                p->red = !p->red;
                g->red = g->parent ? !g->red : BMT_FALSE;
                m->red = !m->red;
                r = g;
            }
            else
            {
                /*
                 *  parent-child combinations:
                 *
                 *  major = true
                 *  minor = false
                 *
                 *  P   C | rotation
                 * -------|----------
                 *  0   0 |	R
                 *  0   1 |	LR
                 *  1   0 |	RL
                 *  1   1 |	L
                 *
                 *	for skews, rotate once
                 *	for triangles, rotate twice
                 */
                BMT_bool pMajor = _bmt_is_major(p);
                BMT_bool nMajor = _bmt_is_major(node);

                if (!pMajor && !nMajor)
                {
                    // R-rotation
                    r = _bmt_rotate_right(node->parent->parent, head);
                }
                else if (!pMajor && nMajor)
                {
                    // LR-rotation
                    r = _bmt_rotate_left(node->parent, head);
                    _bmt_rotate_right(node->parent, head);
                }
                else if (pMajor && !nMajor)
                {
                    // RL-rotation
                    r = _bmt_rotate_right(node->parent, head);
                    _bmt_rotate_left(node->parent, head);
                }
                else
                {
                    // L-rotation
                    r = _bmt_rotate_left(node->parent->parent, head);
                }
                if (r)
                {
                    r->red = !r->red;
                    if (pMajor && r->minor)
                        r->minor->red = !r->minor->red;
                    else if (!pMajor && r->major)
                        r->major->red = !r->major->red;
                }
            }
            _bmt_fix_rbt_violations(r, head);
        }
    }
}

static BMT_node _bmt_rotate_left(BMT_node n, BMT_node* head)
{
    BMT_bool maj = _bmt_is_major(n);
    BMT_node p = n->parent;
    BMT_node m = n->major;
    BMT_node c = m ? n->major->minor : NULL;

    n->parent = m;
    if (m)
        m->minor = n;

    if (c)
        c->parent = n;
    n->major = c;

    if (m)
        m->parent = p;

    p ? (maj ? (p->major = m) : (p->minor = m)) : (*head = m);
    return m;
}

static BMT_node _bmt_rotate_right(BMT_node n, BMT_node* head)
{
    BMT_bool maj = _bmt_is_major(n);
    BMT_node p = n->parent;
    BMT_node m = n->minor;
    BMT_node c = m ? n->minor->major : NULL;

    n->parent = m;
    if (m)
        m->major = n;

    if (c)
        c->parent = n;
    n->minor = c;

    if (m)
        m->parent = p;

    p ? (maj ? (p->major = m) : (p->minor = m)) : (*head = m);
    return m;
}

void _bmt_bst_delete_impl(BMT_node n, BMT_node* _r, BMT_node* _x, BMT_node* _w, BMT_node* head)
{
    if (n)
    {
        BMT_node x = NULL;
        BMT_node w = NULL;
        BMT_node r = NULL;
        BMT_node p = n->parent;

        BMT_bool maj = _bmt_is_major(n);
        BMT_bool root = _bmt_is_root(n);

        if (!n->major && !n->minor)
        {
            if (p)
            {
                maj ? (p->major = NULL) : (p->minor = NULL);
                w = maj ? (p->minor) : (p->major);
            }
        }
        else if ((n->major && !n->minor) || (!n->major && n->minor))
        {
            r = n->major ? n->major : n->minor;
            r->parent = p;

            if (maj)
            {
                if (p)
                    p->major = r;
                x = r->major;
                w = r->minor;
            }
            else
            {
                if (p)
                    p->minor = r;
                x = r->minor;
                w = r->major;
            }
        }
        else
        {
#ifndef BMT_HAVE_BST_MAJOR_INCLINED
            r = _bmt_find_min(n->major);
            w = r->parent ? (_bmt_is_major(r) ? r->parent->minor : r->parent->major) : NULL;

            BMT_node q = n->minor;
            BMT_node m = n->major;
            BMT_node a = r->parent;
            BMT_node b = r->major;

            if (p)
                maj ? (p->major = r) : (p->minor = r);
            r->parent = p;

            if (q)
                q->parent = r;
            r->minor = q;

            if (r != m)
            {
                // link m and r
                r->major = m;
                m->parent = r;

                // link a and b
                a->minor = b;
                if (b)
                    b->parent = a;
            }
#else
            r = _bmt_find_max(n->minor);
            w = r->parent ? (_bmt_is_major(r) ? r->parent->minor : r->parent->major) : NULL;

            BMT_node q = n->major;
            BMT_node m = n->minor;
            BMT_node a = r->parent;
            BMT_node b = r->minor;

            if (p)
                maj ? (p->major = r) : (p->minor = r);
            r->parent = p;

            if (q)
                q->parent = r;
            r->major = q;
            if (r != m)
            {
                // link m and r
                r->minor = m;
                m->parent = r;

                // link a and b
                a->major = b;
                if (b)
                    b->parent = a;
            }
#endif
            x = b;
        }

        *_r = r;
        *_x = x;
        *_w = w;

        if (!p)
            *head = r;

        _bmt_delete_node(n);
    }
}

BMT_bool _bmt_fix_up_rbt(BMT_bool redBefore, BMT_node r, BMT_node x, BMT_node w, BMT_node* head)
{
    BMT_bool redAfter = r ? r->red : BMT_FALSE;
    if (redBefore && redAfter)
        return BMT_TRUE;

    if (!redBefore && redAfter)
    {
        if (r)
            r->red = BMT_FALSE;
        return BMT_TRUE;
    }

    if (redBefore && !redAfter)
    {
        if (r)
            r->red = BMT_TRUE;
    }

    if (_bmt_is_root(x))
        return BMT_TRUE;

    return _bmt_transplant_rbt(x, w, head);
}

static BMT_bool _bmt_transplant_rbt(BMT_node x, BMT_node w, BMT_node* head)
{
    BMT_bool xRed = x ? x->red : BMT_FALSE;
    if (xRed)
    {
        return _bmt_transplant_proc_0(x);
    }
    else
    {
        if (!w)
        {
            // following cases would expect w to have children
            // since double black cannot have children, return here
            return BMT_TRUE;
        }

        BMT_bool maj = !_bmt_is_major(w);
        if (w->red)
        {
            // x is black and w is red
            return _bmt_transplant_proc_1(x, w, head);
        }
        else
        {
            BMT_bool wMjB = w->major ? !w->major->red : BMT_TRUE;
            BMT_bool wMnB = w->minor ? !w->minor->red : BMT_TRUE;

            if (wMjB && wMnB)
            {
                return _bmt_transplant_proc_2(x, w, head);
            }

            BMT_bool c = maj ? (!wMjB && wMnB) : (!wMnB && wMjB);
            if (c)
            {
                return _bmt_transplant_proc_3(x, w, head);
            }

            c = maj ? !wMnB : !wMjB;
            if (c)
            {
                return _bmt_transplant_proc_4(x, w, head);
            }
        }
    }

    return BMT_FALSE;
}

static BMT_bool _bmt_transplant_proc_0(BMT_node x)
{
    if (x->red)
    {
        x->red = BMT_FALSE;
        return BMT_TRUE;
    }

    return BMT_FALSE;
}

static BMT_bool _bmt_transplant_proc_1(BMT_node x, BMT_node w, BMT_node* head)
{
    if (w)
    {
        BMT_bool xBlack = x ? !x->red : BMT_TRUE;
        BMT_bool maj = !_bmt_is_major(w);
        if (xBlack && w->red)
        {
            // only x could be double black, w must be red
            w->red = BMT_FALSE;
            w->parent->red = BMT_TRUE;
            BMT_node m = maj ? w->major : w->minor;
            BMT_node p = maj ? _bmt_rotate_right(w->parent, head) : _bmt_rotate_left(w->parent, head);
            p = maj ? p->major : p->minor;
            x = maj ? p->major : p->minor;
            w = maj ? p->minor : p->major;
            return _bmt_transplant_rbt(x, w, head);
        }
    }
    return BMT_FALSE;
}

static BMT_bool _bmt_transplant_proc_2(BMT_node x, BMT_node w, BMT_node* head)
{
    if (w)
    {
        // x is black and w is black
        // x could be double black, w cannot be double black
        BMT_bool xBlack = x ? !x->red : BMT_TRUE;
        if (xBlack && !w->red)
        {
            // only x could be double black
            BMT_bool maj = !_bmt_is_major(w);
            BMT_bool wMnB = w->minor ? !w->minor->red : BMT_TRUE;
            BMT_bool wMjB = w->major ? !w->major->red : BMT_TRUE;
            if (wMnB && wMjB)
            {
                w->red = BMT_TRUE;
                x = w->parent;
                if (_bmt_is_root(x))
                {
                    x->red = BMT_FALSE;
                    return BMT_TRUE;
                }
                maj = _bmt_is_major(x);
                w = maj ? x->parent->minor : x->parent->major;
                if (x->red)
                {
                    x->red = BMT_FALSE;
                    return BMT_TRUE;
                }
                else
                {
                    w = maj ? w->parent->minor : w->parent->major;
                    x = maj ? w->parent->major : w->parent->minor;
                    return _bmt_transplant_rbt(x, w, head);
                }
            }
        }
    }
    return BMT_FALSE;
}

static BMT_bool _bmt_transplant_proc_3(BMT_node x, BMT_node w, BMT_node* head)
{
    if (w)
    {
        BMT_bool xBlack = x ? !x->red : BMT_TRUE;
        BMT_bool wMjB = w->major ? !w->major->red : BMT_TRUE;
        BMT_bool wMnB = w->minor ? !w->minor->red : BMT_TRUE;
        BMT_bool maj = !_bmt_is_major(w);
        BMT_bool c = maj ? (!wMjB && wMnB) : (wMjB && !wMnB);

        if (xBlack && c)
        {
            maj = !_bmt_is_major(w);
            maj ? (w->major->red = BMT_FALSE) : (w->minor->red = BMT_FALSE);
            w->red = BMT_TRUE;
            w = maj ? _bmt_rotate_left(w, head) : _bmt_rotate_right(w, head);
            x = maj ? w->parent->major : w->parent->minor;
            return _bmt_transplant_proc_4(x, w, head);
        }
    }
    return BMT_FALSE;
}

static BMT_bool _bmt_transplant_proc_4(BMT_node x, BMT_node w, BMT_node* head)
{
    if (w)
    {
        // x can be double black
        // w cannot be double black
        BMT_bool wMjR = w->major ? w->major->red : BMT_FALSE;
        BMT_bool wMnR = w->minor ? w->minor->red : BMT_FALSE;
        BMT_bool xBlack = x ? !x->red : BMT_TRUE;
        BMT_bool maj = !_bmt_is_major(w);
        BMT_bool c = maj ? wMnR : wMjR;

        if (xBlack && c)
        {
            w->red = w->parent->red;
            w->parent->red = BMT_FALSE;
            maj ? (w->minor->red = BMT_FALSE) : (w->major->red = BMT_FALSE);
            maj ? _bmt_rotate_right(w->parent, head) : _bmt_rotate_left(w->parent, head);
            return BMT_TRUE;
        }
    }
    return BMT_FALSE;
}
