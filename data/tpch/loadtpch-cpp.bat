@echo off
set admin=..\..\bin\admin
%admin% initdb tpch
%admin% createtable tpch customer c_custkey key int c_name string c_address string c_nationkey int c_phone string c_acctabl double c_mktsegment string c_comment string
%admin% createtable tpch lineitem l_orderkey int l_partkey int l_suppkey int l_linenumber int l_quantity int l_extendedprice double l_discount double l_tax double l_returnflag string l_linestatus string l_shipdate string l_commitdate string l_recepitdate string l_shipstruct string l_shipmode string l_comment string
%admin% createtable tpch nation n_nationkey key int n_name string n_regionkey int n_comment string
%admin% createtable tpch orders o_orderkey key int o_custkey int o_orderstatus string o_totalprice double o_orderdate string o_orderpriority string o_clerk string o_shippingpriority int o_comment string
%admin% createtable tpch part p_partkey key int p_name string p_mfgr string p_brand string p_type string p_size int p_container string p_retailprice double p_comment string
%admin% createtable tpch partsupp ps_partkey int ps_suppkey int ps_availqty int ps_supplycosts double ps_comment string
%admin% createtable tpch region r_regionkey key int r_name string r_commane string
%admin% createtable tpch supplier s_suppkey key int s_name string s_address string s_nationkey int s_phone string s_acctbal double s_comment string
%admin% bulkload tpch customer customer.tbl
%admin% bulkload tpch lineitem lineitem.tbl
%admin% bulkload tpch nation nation.tbl
%admin% bulkload tpch orders orders.tbl
%admin% bulkload tpch part part.tbl
%admin% bulkload tpch partsupp partsupp.tbl
%admin% bulkload tpch region region.tbl
%admin% bulkload tpch supplier supplier.tbl
%admin% runstats tpch

