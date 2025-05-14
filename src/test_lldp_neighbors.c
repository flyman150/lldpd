/**
 * @file test_lldp_neighbors.c
 * @brief 使用 liblldpctl 库查看所有网络邻居信息的测试程序
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lldpctl.h>

/**
 * @brief 打印邻居信息
 * @param port 端口信息
 * @param chassis 机箱信息
 */
static void print_neighbor_info(lldpctl_atom_t *port, lldpctl_atom_t *chassis) {
    const char *port_name = lldpctl_atom_get_str(port, lldpctl_k_port_name);
    const char *port_descr = lldpctl_atom_get_str(port, lldpctl_k_port_descr);
    const char *chassis_name = lldpctl_atom_get_str(chassis, lldpctl_k_chassis_name);
    const char *chassis_descr = lldpctl_atom_get_str(chassis, lldpctl_k_chassis_descr);
    long int chassis_cap = lldpctl_atom_get_int(chassis, lldpctl_k_chassis_cap_enabled);

    printf("\n=== 邻居信息 ===\n");
    printf("端口名称: %s\n", port_name ? port_name : "未知");
    printf("端口描述: %s\n", port_descr ? port_descr : "未知");
    printf("设备名称: %s\n", chassis_name ? chassis_name : "未知");
    printf("设备描述: %s\n", chassis_descr ? chassis_descr : "未知");
    printf("设备能力: 0x%lx\n", chassis_cap);

    // 获取管理地址
    lldpctl_atom_t *mgmt = lldpctl_atom_get(port, lldpctl_k_port_chassis);
    if (mgmt) {
        lldpctl_atom_t *mgmt_addr = lldpctl_atom_get(mgmt, lldpctl_k_chassis_mgmt);
        if (mgmt_addr) {
            const char *ip = lldpctl_atom_get_str(mgmt_addr, lldpctl_k_mgmt_ip);
            if (ip) {
                printf("管理地址: %s\n", ip);
            }
            lldpctl_atom_dec_ref(mgmt_addr);
        }
        lldpctl_atom_dec_ref(mgmt);
    }
}

int main(int argc, char *argv[]) {
    lldpctl_conn_t *conn = NULL;
    lldpctl_atom_t *interfaces = NULL;
    lldpctl_atom_t *interface = NULL;
    lldpctl_atom_t *neighbors = NULL;
    lldpctl_atom_t *neighbor = NULL;
    lldpctl_atom_t *chassis = NULL;
    int ret = 1;

    // 创建连接
    conn = lldpctl_new_name(NULL, NULL, NULL, NULL);
    if (!conn) {
        fprintf(stderr, "无法连接到 lldpd: %s\n", lldpctl_strerror(lldpctl_last_error(NULL)));
        goto end;
    }

    // 获取所有接口
    interfaces = lldpctl_get_interfaces(conn);
    if (!interfaces) {
        fprintf(stderr, "无法获取接口列表: %s\n", lldpctl_strerror(lldpctl_last_error(conn)));
        goto end;
    }

    // 遍历所有接口
    while ((interface = lldpctl_atom_iterate(interfaces, interface)) != NULL) {
        const char *ifname = lldpctl_atom_get_str(interface, lldpctl_k_interface_name);
        printf("\n接口: %s\n", ifname ? ifname : "未知");

        // 获取该接口的所有邻居
        neighbors = lldpctl_atom_get(interface, lldpctl_k_port_neighbors);
        if (!neighbors) {
            fprintf(stderr, "无法获取邻居列表: %s\n", lldpctl_strerror(lldpctl_last_error(conn)));
            continue;
        }

        // 遍历所有邻居
        while ((neighbor = lldpctl_atom_iterate(neighbors, neighbor)) != NULL) {
            // 获取邻居的机箱信息
            chassis = lldpctl_atom_get(neighbor, lldpctl_k_port_chassis);
            if (chassis) {
                print_neighbor_info(neighbor, chassis);
                lldpctl_atom_dec_ref(chassis);
            }
        }

        lldpctl_atom_dec_ref(neighbors);
    }

    ret = 0;

end:
    if (interfaces) lldpctl_atom_dec_ref(interfaces);
    if (conn) lldpctl_release(conn);
    return ret;
} 